#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define VERBOSE
#include "debug.h"
#include "CameraConnection.h"

#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

#ifdef HAVE_LIBB64
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
    DEBUG("Built new camera connection");
    stream << "select_camera " << camera << std::endl;
    DEBUG("Trying to read response");
    std::string response;
    std::getline( stream, response );
    DEBUG("Stream status is " << bool(stream) << " and response " << response);
}

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

void CameraConnection::start_acquisition( input::Traits<engine::Image>& traits )
{
    traits.image_number().range().first = 0 * camera::frame;
    stream << "start_acquisition\n";
    declaration_parser<std::string::const_iterator> p(traits);
    while ( true ) {
        std::string line;
        getline(stream, line);
        DEBUG("Traits loop processing line " << line);
        std::string::const_iterator b = line.begin(), e = line.end();
        bool go_on = true, good;
        good = phrase_parse( b, e, p, qi::eps, qi::skip_flag::dont_postskip, go_on );
        DEBUG("Parsing result: " << good << " " << (b==e) << " " << go_on);
        DEBUG("Width is " << traits.size.x() << " at " << &traits.size.x());
        DEBUG("Height is " << traits.size.y() << " at " << &traits.size.y());
        if ( good && b == e && ! go_on ) break;
    }
}

void CameraConnection::set_traits( input::Traits<engine::Image>& traits )
{
    traits.image_number().range().first = 0 * camera::frame;
    traits.image_number().resolution().promise( dStorm::deferred::JobTraits );

#if 0
    stream << "will_have_length\n";
    std::string line;
    getline(stream, line);
    if ( line == "true\n" || line == "true\r\n" || line == "true" )
        traits.image_number().range().second.promise( dStorm::deferred::JobTraits );
#endif
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
        final = good | bad | ugly;
    }
};

CameraConnection::FrameFetch CameraConnection::next_frame() {
    parser< std::string::const_iterator > p;
    std::string line;
    getline(stream, line);
    if ( ! stream ) throw std::runtime_error("Premature end of Andor camera data stream");
    DEBUG("Main loop processing line " << line);
    std::string::const_iterator b = line.begin(), e = line.end();
    FrameFetch rv;
    bool good = phrase_parse( b, e, p, ascii::space, rv );
    if ( good && b == e )
        return rv;
    else
        throw std::runtime_error("Invalid command: '" + line + "'");
}

void CameraConnection::read_data( CamImage& img ) {
    DEBUG("Should read image of size " << img.size_in_pixels());
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
        throw std::logic_error("Length mismatch in base64 decoding");
}

}
}
