#include "Attribute.hh"
#include <math.h>
#include <limits>

namespace simparm {

std::ostream& to_config_stream( std::ostream& o, bool b )
{
    return (o << std::boolalpha << b);
}

std::istream& from_config_stream( std::istream& i, bool& b )
{
    (i >> std::boolalpha >> b);
    if ( !i ) {
        std::string s;
        std::getline(i, s);
        throw std::runtime_error( "Invalid boolean value " + s );
    } else
        return i;
}

template <typename Type> 
inline std::ostream& output( std::ostream& o, Type v ) {
    typedef std::numeric_limits<long double> Lim;
    switch (fpclassify(v)) {
        case FP_NAN: return (o << "NaN");
        case FP_INFINITE: return (o << ((v > 0) ? "inf" : "-inf"));
        default:
            return (o << v);
    }
}
template <typename Type> 
inline std::istream& input( std::istream& i, Type& v ) {
    return (i >> v);
}


/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream& i, float& f ) 
    { return input(i,f); }
/** Writes true and false instead of 1 and 0. */
std::ostream& to_config_stream( std::ostream& o, float f ) 
    { return output(o, f); }
/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream& i, double& d )
    { return input(i, d); }
/** Writes true and false instead of 1 and 0. */
std::ostream& to_config_stream( std::ostream& o, double d )
    { return output(o, d); }
/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream& i, long double& d )
    { return input(i, d); }
/** Writes true and false instead of 1 and 0. */
std::ostream& to_config_stream( std::ostream& o, long double d )
    { return output(o, d); }

/** Reads one line as string, terminating at either carriage
 *  return or newline character. */
std::istream& from_config_stream( std::istream& i, std::string& rv ) {
    rv.clear();
    char c;
    i.get(c);   /* eat leading space. */
    if ( i && c != ' ' ) rv = c;
    while ( (i.get(c)) && c != '\n' && c != '\r' ) { rv += c; }
    return i; 
}

}
