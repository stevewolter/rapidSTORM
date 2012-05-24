#ifndef SIMPARM_BOOST_UNITS_HPP
#define SIMPARM_BOOST_UNITS_HPP

#include <boost/units/quantity.hpp>
#include <math.h>
#include <boost/units/io.hpp>
#include <boost/units/cmath.hpp>
#include "iostream.h"
#include "default_value.h"

namespace simparm {

template <typename Unit, typename Value>
inline boost::units::quantity<Unit,Value> default_value( const boost::units::quantity<Unit,Value>& ) { 
    return boost::units::quantity<Unit,Value>::from_value(
        default_value( Value() ) );
}

template <typename Unit, typename Value>
inline boost::units::quantity<Unit,Value> default_increment( const boost::units::quantity<Unit,Value>& ) { 
    return boost::units::quantity<Unit,Value>::from_value( default_increment( Value() ) );
}

template <typename Unit, typename Value>
inline const char *typeName( boost::units::quantity<Unit,Value> ) { return typeName(Value()); }

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

template <typename Unit, typename Value>
inline bool exceeds( const boost::units::quantity<Unit,Value>& a, const boost::units::quantity<Unit,Value>& b ) {
    return a > b;
}

template <typename Unit, typename Value>
inline bool falls_below( const boost::units::quantity<Unit,Value>& a, const boost::units::quantity<Unit,Value>& b ) {
    return a < b;
}

}

#include "Attribute.h"
#include "Entry.h"
#include "Attributes.h"

namespace simparm {

template <typename Unit, typename Value>
inline bool ensure_lower_bound( boost::units::quantity<Unit,Value>& a, const boost::units::quantity<Unit,Value>& bound )
{
    bool changed = (a < bound);
    if ( changed ) a = bound;
    return changed;
}

template <typename Unit, typename Value>
inline bool ensure_upper_bound( boost::units::quantity<Unit,Value>& a, const boost::units::quantity<Unit,Value>& bound )
{
    bool changed = (a > bound);
    if ( changed ) a = bound;
    return changed;
}

}

namespace simparm {

template <typename Unit, typename Value, typename Field>
struct Attributes< boost::units::quantity<Unit,Value>, Field, void >
: public Attributes< Value, Field, void >
{
    typedef Attributes<Value,Field,void> Base;
    Attributes( Attribute<Field>& e ) 
        : Base(e), long_unit("unit_name", name_string(Unit()) ), 
          short_unit( "unit_symbol", symbol_string(Unit()) ) {}
    Attributes( const Attributes& a, Attribute<Field>& e ) 
        : Base(a, e), long_unit(a.long_unit), short_unit(a.short_unit) {}
    void registerNamedEntries( simparm::NodeHandle n ) {
        long_unit.add_to( n );
        short_unit.add_to( n );
        Base::registerNamedEntries(n);
    }
  private:
   Attribute<std::string> long_unit, short_unit;
};

}

#endif
