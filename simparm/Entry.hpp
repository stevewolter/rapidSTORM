#ifndef SIMPARM_EntryTypeImpl
#define SIMPARM_EntryTypeImpl

#include "Entry.h"
#include "iostream.h"
#include "typeName.h"
#include "is_numerical.hpp"

namespace simparm {

template <typename Type>
typename boost::enable_if< boost::is_integral<Type>, void >::type
inline add_attributes( Type, boost::ptr_vector< BaseAttribute >& ) {}
template <typename Type>
typename boost::enable_if< boost::is_floating_point<Type>, void >::type
inline add_attributes( Type, boost::ptr_vector< BaseAttribute >& ) {}

inline void add_attributes( std::string, boost::ptr_vector< BaseAttribute >& ) {}

template <typename Unit, typename Value> 
inline void add_attributes( boost::units::quantity<Unit,Value>, boost::ptr_vector< BaseAttribute >& );
template <typename Inner>
inline void add_attributes( boost::optional<Inner>, boost::ptr_vector< BaseAttribute >& t );
template <typename Derived>
inline void add_attributes( Eigen::MatrixBase<Derived> d, boost::ptr_vector< BaseAttribute >& t );

template <typename Unit, typename Value> 
inline void add_attributes( boost::units::quantity<Unit,Value>, boost::ptr_vector< BaseAttribute >& t )
{
    add_attributes( Value(), t );
    t.push_back( new Attribute<std::string>("unit_name", boost::units::name_string(Unit()) ) );
    t.push_back( new Attribute<std::string>("unit_symbol", boost::units::symbol_string(Unit()) ) );
}

template <typename Inner>
void add_attributes( boost::optional<Inner>, boost::ptr_vector< BaseAttribute >& t ) {
    add_attributes( Inner(), t );
    t.push_back( new Attribute<bool>("is_optional", true) );
}
template <typename Derived>
void add_attributes( Eigen::MatrixBase<Derived> d, boost::ptr_vector< BaseAttribute >& t ) {
    add_attributes( typename Derived::Scalar(), t );
    t.push_back( new Attribute<int>("rows", Derived().rows()) );
    t.push_back( new Attribute<int>("columns", Derived().cols()) );
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry(
    string name, string desc, const TypeOfEntry& startVal)
: BasicEntry(name,desc), 
  value( "value", startVal ),
  increment("increment", startVal ),
  min("min", bound_type() ),
  max("max", bound_type() )
{
    add_attributes( TypeOfEntry(), additional_attributes );
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry(
    const Entry<TypeOfEntry>& from)
: BasicEntry(from), 
  value( from.value ),
  increment(from.increment),
  min(from.min),
  max(from.max)
{
    add_attributes( TypeOfEntry(), additional_attributes );
}

template <typename TypeOfEntry>
NodeHandle Entry<TypeOfEntry>::make_naked_node( simparm::NodeHandle node ) {
    if ( boost::is_same< TypeOfEntry, bool >() )
        return create_checkbox( node, getName() );
    else
        return create_textfield( node, getName(), typeName( TypeOfEntry() ) ); 
}

template <typename TypeOfEntry>
NodeHandle
Entry<TypeOfEntry>::create_hidden_node(simparm::NodeHandle to) {
    NodeHandle r = BasicEntry::create_hidden_node(to);
    std::for_each( additional_attributes.begin(), additional_attributes.end(),
        boost::bind( &BaseAttribute::add_to, _1, r ) );
    if ( is_numerical( TypeOfEntry() ) ) {
        increment.add_to(r);
        min.add_to(r);
        max.add_to(r);
    }
    value.add_to(r);
    return r;
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::~Entry() {}

}

#endif
