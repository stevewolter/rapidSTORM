#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "CameraConnection.h"

#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#include <boost/lexical_cast.hpp>

extern "C" {
#include <b64/cdecode.h>
}

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

#if 0
template <typename Type>
struct remove_optional { typedef Type type; };
template <typename Type>
struct remove_optional< dStorm::optional<Type> > { typedef Type type; };

struct from_value {
    template <typename Left, typename Right>
    struct result { typedef bool type; };

    template <typename Left, typename Right>
    bool operator()( Left assign, Right arg ) const {
        DEBUG( "Assigning " << arg << " to " << &assign );
        assign = remove_optional<Left>::type::from_value(arg);
        return true;
    }
};

template <typename Iterator>
struct declaration_parser : qi::grammar<Iterator, bool()>
{
    struct assign_width {
        input::Traits<engine::Image>& traits;
        assign_width( input::Traits<engine::Image>& traits ) : traits(traits) {}
        void operator()( int i, boost::spirit::unused_type, boost::spirit::unused_type ) const { traits.size.x() = i * camera::pixel; DEBUG("Written width"); }
    };

    struct assign_height {
        input::Traits<engine::Image>& traits;
        assign_height( input::Traits<engine::Image>& traits ) : traits(traits) {}
        void operator()( int i, boost::spirit::unused_type, boost::spirit::unused_type ) const { traits.size.y() = i * camera::pixel; DEBUG("Written height"); }
    };

    struct assign_imnum {
        input::Traits<engine::Image>& traits;
        assign_imnum( input::Traits<engine::Image>& traits ) : traits(traits) {}
        void operator()( int i, boost::spirit::unused_type, boost::spirit::unused_type ) const { traits.image_number().range().second = i * camera::frame; DEBUG("Written imnum"); }
    };

#define ASSIGN(x) _val = phx::function<from_value>()( phx::ref(traits. x) , _1 )
    declaration_parser( input::Traits<engine::Image>& traits )
    : declaration_parser::base_type(final) , traits(traits)
    {
        namespace phx = boost::phoenix;
        using qi::labels::_val;
        using qi::labels::_1;
        length = ("length " >> qi::int_)[assign_imnum(traits)][_val = phx::val(true)];
        width = ("width " >> qi::int_)[assign_width(traits)][_val = phx::val(true)];
        height = ("height " >> qi::int_)[assign_height(traits)][_val = phx::val(true)];
        acquisition = qi::lit("acquisition has started")[_val = phx::val(false)];
        final = acquisition[_val = _1] | length[_val = _1] | width[_val = _1] | height[_val = _1];
    }
    qi::rule<Iterator, bool()> acquisition, final, length, height, width;
    input::Traits<engine::Image>& traits;
};
#endif

void CameraConnection::start_acquisition( CamTraits& traits, simparm::StringEntry& status )
{
    DEBUG("Sending start acquisition command");
    traits.image_number().range().first = 0 * camera::frame;
    traits.size.fill( 1 * camera::pixel );
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
            traits.size.x() = (boost::lexical_cast<int>(args)) * camera::pixel;
        } else if ( command == "height" ) {
            traits.size.y() = (boost::lexical_cast<int>(args)) * camera::pixel;
        } else if ( command == "acquisition" && args == "has started") {
            break;
        }
    }
    DEBUG("Acquisition started");
}

void CameraConnection::set_traits( input::Traits<engine::Image>& traits )
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

    base64_decodestate state;
    base64_init_decodestate(&state);
    int codedlength = base64_decode_block( 
        buffer.get(), brokenlineslength-1, reinterpret_cast<char*>(img.ptr()), &state );
    if ( codedlength != decodelength ) 
        throw std::logic_error("Length mismatch in base64 decoding: Tried to decode " +
            boost::lexical_cast<std::string>( decodelength ) + ", but decoded " +
            boost::lexical_cast<std::string>( codedlength ) );
    DEBUG("Did read image");
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
