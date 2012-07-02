/** @file simparm/iostream.h 
 *        Input/output operators different from standard template 
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
#include <boost/optional/optional.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/io.hpp>
#include <Eigen/Core>

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
        if ( i.peek() == '-' ) { return i; }
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

template <typename Inner>
inline std::istream& from_config_stream( std::istream& i, boost::optional<Inner>& t ); 
template <typename Inner>
inline std::ostream& to_config_stream( std::ostream& i, const boost::optional<Inner>& t );

template <typename Unit, typename Value>
inline std::istream& from_config_stream( std::istream& i, boost::units::quantity<Unit,Value>& t );
template <typename Unit, typename Value>
inline std::ostream& to_config_stream( std::ostream& o, const boost::units::quantity<Unit,Value>& t );

template <typename Derived>
std::istream& from_config_stream( std::istream& i, Eigen::MatrixBase<Derived>& t );
template <typename Derived>
std::ostream& to_config_stream( std::ostream& o, const Eigen::MatrixBase<Derived>& t );

/** Reads true and false instead of 1 and 0. */
std::istream& from_config_stream( std::istream&, bool& );
/** Writes true and false instead of 1 and 0. */
std::ostream& to_config_stream( std::ostream&, bool );

/** Reads NaN and infinity for special values. */
std::istream& from_config_stream( std::istream&, float& );
/** Writes NaN and infinity for special values. */
std::ostream& to_config_stream( std::ostream&, float );

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

template <typename Inner>
inline std::istream& from_config_stream( std::istream& i, boost::optional<Inner>& t )
{ 
    t = Inner();
    return from_config_stream(i, *t);
}

template <typename Inner>
inline std::ostream& to_config_stream( std::ostream& i, const boost::optional<Inner>& t ) { 
    return to_config_stream(i, *t);
}

template <typename Unit, typename Value>
inline std::istream& from_config_stream( std::istream& i, boost::units::quantity<Unit,Value>& t ) {
    Value v;
    std::istream& iv = from_config_stream(i, v);
    t = boost::units::quantity<Unit,Value>::from_value(v);
    return iv;
}

template <typename Unit, typename Value>
inline std::ostream& to_config_stream( std::ostream& o, const boost::units::quantity<Unit,Value>& t ) { 
    return to_config_stream( o, t.value() );
}

template <typename Derived>
std::istream& from_config_stream( std::istream& i, Eigen::MatrixBase<Derived>& t ) { 
   char sep;
   i >> std::skipws;
   for ( int r = 0; r < t.rows(); ++r ) {
        if ( r != 0 ) {
            i >> sep;
            if ( !i || sep != ',' ) 
                throw std::runtime_error("No comma found separating matrix entries");
        }
        for ( int c = 0; c < t.cols(); ++c )
            from_config_stream(i, t(r,c) );
   }
   return i;
}

template <typename Derived>
std::ostream& to_config_stream( std::ostream& o, const Eigen::MatrixBase<Derived>& t ) { 
    for (int r = 0; r < t.rows(); ++r) {
        if ( r > 0 ) o << ",";
        for (int c = 0; c < t.cols(); ++c) {
            if ( c > 0 ) o << " ";
            to_config_stream(o, t(r,c));
        }
    }
    return o;
}

template <typename Type>
inline bool value_is_given( const Type& a )  { return true; }
template <typename Inner>
inline bool value_is_given( const boost::optional<Inner>& a ) 
    { return a; }


}

#endif
