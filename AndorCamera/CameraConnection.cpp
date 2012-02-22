#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "CameraConnection.h"
#include <dStorm/engine/InputTraits.h>

#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/lexical_cast.hpp>

#if HAVE_LIBB64
extern "C" {
#include <b64/cdecode.h>
}
#endif

namespace dStorm {
namespace AndorCamera {

using namespace boost::asio;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;


CameraConnection::CameraConnection( const std::string& hostname, int camera, const std::string& port )
: stream(hostname, port)
{
    DEBUG("Built new camera connection " << this);
    stream << "simparm attach\nselect_camera " << camera << std::endl;
    DEBUG("Trying to read response");
    std::string response;
    std::getline( stream, response );
    DEBUG("Stream status is " << bool(stream) << " and response " << response);
}

CameraConnection::~CameraConnection() {
    DEBUG("Destructing camera connection " << this);
}

void CameraConnection::start_acquisition( CamTraits& traits, simparm::StringEntry& status )
{
    DEBUG("Sending start acquisition command");
    traits.image_number().range().first = 0 * camera::frame;
    dStorm::image::MetaInfo<2> size;
    size.size.fill( 1 * camera::pixel );
    traits.push_back( size, dStorm::traits::Optics() );
    stream << "start_acquisition" << std::endl;
    while ( true ) {
        DEBUG("Reading acquisition start line in " << this);
        std::string line;
        getline(stream, line);
        DEBUG("Read acquisition start line in " << this);
        if ( ! stream ) throw std::runtime_error("Unable to connect to camera");
        DEBUG("Traits loop processing line " << line);
        std::string command = line.substr(0, line.find(' ')),
                    args = line.substr( line.find(' ')+1 );
        if ( command == "simparm" ) {
            parse_simparm(line);
        } else if ( command == "status" ) {
            status = args;
        } else if ( command == "error" ) {
            throw std::runtime_error( args );
        } else if ( line[0] == '2' ) {
            /* Some success code. Just go on. */
        } else if ( command == "length" ) {
            traits.image_number().range().second = (boost::lexical_cast<int>(args)-1) * camera::frame;
        } else if ( command == "width" ) {
            traits.image(0).size.x() = (boost::lexical_cast<int>(args)) * camera::pixel;
            if ( traits.image(0).size.x() >= 10240 * camera::pixel || traits.image(0).size.x() <= 0 * camera::pixel )
                throw std::runtime_error("Camera sent bogus width " + args);
        } else if ( command == "height" ) {
            traits.image(0).size.y() = (boost::lexical_cast<int>(args)) * camera::pixel;
            if ( traits.image(0).size.y() >= 10240 * camera::pixel || traits.image(0).size.y() <= 0 * camera::pixel )
                throw std::runtime_error("Camera sent bogus height " + args);
        } else if ( command == "acquisition" && args == "has started") {
            break;
        }
    }
    DEBUG("Acquisition started");
}

void CameraConnection::set_traits( CamTraits& traits )
{
    traits.image_number().range().first = 0 * camera::frame;
}

template <typename Type>
struct with_frame_number {
    template <typename Right>
    struct result { typedef Type type; };

    template <typename Right>
    Type operator()( Right arg ) const {
        Type t;
        t.frame_number = arg * boost::units::camera::frame;
        return t;
    }
};

template <typename Iterator>
struct parser : qi::grammar<Iterator, CameraConnection::FrameFetch()>
{
    qi::rule<Iterator, CameraConnection::FetchImage()> good;
    qi::rule<Iterator, CameraConnection::ImageError()> bad;
    qi::rule<Iterator, CameraConnection::EndOfAcquisition()> ugly;
    qi::rule<Iterator, CameraConnection::FrameFetch()> final;

    parser() : parser::base_type(final)
    {
        namespace phx = boost::phoenix;
        using qi::labels::_val;
        using qi::labels::_1;
        good = ("image " >> qi::int_ >> " data base64")[_val = phx::function< with_frame_number<CameraConnection::FetchImage> >()(_1) ];
        bad = ("image " >> qi::int_ >> " error")[_val = phx::function< with_frame_number<CameraConnection::ImageError> >()(_1) ];
        ugly = qi::lit("acquisition ended")[_val = phx::val(CameraConnection::EndOfAcquisition()) ];
        final = good[_val = _1] | bad[_val = _1] | ugly[_val = _1];
    }
};

CameraConnection::FrameFetch CameraConnection::next_frame() {
    parser< std::string::const_iterator > p;
    DEBUG("Reading next frame line in " << this);
    std::string line;
    DEBUG("Read next frame line in " << this);
    getline(stream, line);
    if ( ! stream ) throw std::runtime_error("Premature end of Andor camera data stream");
    DEBUG("Main loop processing line " << line);
    if ( line.substr(0, 8) == "simparm " ) {
        return parse_simparm(line);
    } else if ( line.substr(0, 7) == "status " ) {
        return StatusChange( line.substr(7) );
    } else {
        std::string::const_iterator b = line.begin(), e = line.end();
        FrameFetch rv;
        bool good = phrase_parse( b, e, p, ascii::space, rv );
        if ( good && b == e ) {
            return rv;
        } else
            throw std::runtime_error("Invalid command: '" + line + "'");
    }
}

void CameraConnection::read_data( CamImage& img ) {
    DEBUG("Should read image number " << img.frame_number() << " of size " << img.size_in_pixels());
    int pixcount = img.size_in_pixels();
    int decodelength = pixcount * 2;
    int codelength = (decodelength / 3) * 4 + ((decodelength % 3) ? 4 : 0);
    int numlines = codelength / 72;
    int brokenlineslength = codelength + numlines + 2;

    boost::shared_array<char> buffer( new char[brokenlineslength] );
    stream.read( buffer.get(), brokenlineslength );

#if HAVE_LIBB64
    base64_decodestate state;
    base64_init_decodestate(&state);
    int codedlength = base64_decode_block( 
        buffer.get(), brokenlineslength-1, reinterpret_cast<char*>(img.ptr()), &state );
    if ( codedlength != decodelength ) 
        throw std::logic_error("Length mismatch in base64 decoding: Tried to decode " +
            boost::lexical_cast<std::string>( decodelength ) + ", but decoded " +
            boost::lexical_cast<std::string>( codedlength ) );
    DEBUG("Did read image");
#else
    throw std::runtime_error("Andor camera support compiled without Base 64 support, cannot read data");
#endif
}

void CameraConnection::send( const std::string& s ) {
    stream << "simparm " << s << std::endl;
}

void CameraConnection::stop_acquisition() {
    stream << "stop_acquisition" << std::endl;
}


CameraConnection::Simparm CameraConnection::parse_simparm(std::string line) {
    Simparm rv;
    std::string& m = rv.message;
    int open_declarations = 0;
    std::string next_line = line.substr(8);
    do {
        if ( next_line == "" ) std::getline( stream, next_line );
        DEBUG("Scanning line " << next_line << " for simparm info");
        std::stringstream ss(next_line);
        while ( ss ) {
            std::string cmd;
            ss >> cmd;
            if ( cmd == "in" ) { ss >> cmd; continue; } 
            else if ( cmd == "declare") ++open_declarations;
            else if ( cmd == "end") --open_declarations;
            break;
        }
        m += next_line + "\n";
        next_line = "";
    } while ( open_declarations > 0 );
    return rv;
}

}
}
