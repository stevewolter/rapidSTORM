/** @file Input/output operators different from standard template 
 *        library where necessary. Implemented in Attribute.cc. */
#ifndef SIMPARM_IOSTREAM_HH
#define SIMPARM_IOSTREAM_HH

#include <iostream>
#include <stdexcept>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/type_traits/is_floating_point.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_enum.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/or.hpp>

#include <boost/type_traits/is_unsigned.hpp>

namespace simparm {

/** The from_config_stream converts a value from the control stream
 *  representation back to memory representation.
 *
 *  The standard, generic version is implemented as an alias for
 *  operator>>. */
template <typename Type>
inline typename boost::enable_if< 
    boost::mpl::or_< boost::is_fundamental<Type>, boost::is_enum<Type> >,
    std::istream& >::type
from_config_stream( std::istream& i, Type& t ) { 
    /* Work around mis-specification in C++ that allows reading -1 with an integer overflow */
    if ( boost::is_unsigned<Type>::value ) {
        while ( isspace( i.peek() ) ) i.get();
        if ( i.peek() == '-' ) {
            throw std::runtime_error("Only positive numbers allowed here");
        }
    }
    std::istream& rv = (i >> t); 
    return rv;
}

/** The from_config_stream converts a value from its memory representation
 *  to a character stream suitable for transportation over the control
 *  stream.
 *
 *  The standard, generic version is implemented as an alias for
 *  operator<<. */
template <typename Type>
inline typename boost::enable_if< 
    boost::mpl::or_< boost::is_fundamental<Type>, boost::is_enum<Type> >,
    std::ostream& >::type
to_config_stream( std::ostream& o, const Type& t )
    { return (o << t); }
inline std::ostream& to_config_stream( std::ostream& o, const std::string& s )
    { return (o << s); }

/** Reads true and false instead of 1 and 0. */
std::istream& from_config_stream( std::istream&, bool& );
/** Writes true and false instead of 1 and 0. */
std::ostream& to_config_stream( std::ostream&, bool );
inline const char *typeName( const bool &) { return "Bool"; }

/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream&, float& );
/** Writes NaN and infinity for special values. */
std::ostream& to_config_stream( std::ostream&, float );

template <typename Type>
inline typename boost::enable_if< boost::is_integral<Type>, const char * >::type
typeName(const Type&) { return "Integer"; }
template <typename Type>
inline typename boost::enable_if< boost::is_floating_point<Type>, const char * >::type
typeName(const Type&) { return "Float"; }

/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream&, double& );
/** Writes NaN and infinity for special values. */
std::ostream& to_config_stream( std::ostream&, double );

/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream&, long double& );
/** Writes NaN and infinity for special values. */
std::ostream& to_config_stream( std::ostream&, long double );

/** Reads one line as string, terminating at either carriage
 *  return or newline character. */
std::istream& from_config_stream( std::istream&, std::string& );
inline const char *typeName( const std::string& ) { return "String"; }

}

#endif
