#ifndef SIMPARM_EntryTypeImpl
#define SIMPARM_EntryTypeImpl

#include "Entry.h"
#include "iostream.h"
#include "typeName.h"
#include "is_numerical.hpp"
#include "falls_below.hpp"
#include "exceeds.hpp"

namespace simparm {

template <typename Bounder, typename Boundee, bool LowerBound>
class OneBoundWatcher
: public Attribute< Boundee >::ChangeWatchFunction,
  private boost::noncopyable
{
  private:
    const Attribute< Bounder > &bound;

  public:
    OneBoundWatcher( const Attribute<Bounder>& value )
        : bound(value) { }

    /* The parameters are not used since we don't know whether we get 
     * the callback from value, min or max */
    bool operator()(const Boundee& from, const Boundee& to) {
        return ! ( (LowerBound) ? exceeds(bound(), to) : falls_below(bound(), to) );
    }
};

template <typename Bounder, typename Boundee>
class BothBoundsWatcher 
: public Attribute< Boundee >::ChangeWatchFunction, private boost::noncopyable
{
private:
    OneBoundWatcher<Bounder,Boundee,true> lower;
    OneBoundWatcher<Bounder,Boundee,false> upper;
public:
    BothBoundsWatcher( const Attribute<Bounder>& lower_bound, const Attribute<Bounder>& upper_bound )
        : lower( lower_bound ), upper( upper_bound ) {}
    bool operator()(const Boundee& from, const Boundee& to) {
        return lower( from, to ) && upper( from, to );
    }
};

template <typename BoundType, typename ValueType>
inline std::auto_ptr< typename Attribute<ValueType>::ChangeWatchFunction >
create_bounds_watcher( 
    const Attribute<BoundType>& min, 
    Attribute<ValueType>& value, 
    const Attribute<BoundType>& max 
) {
    std::auto_ptr< typename Attribute<ValueType>::ChangeWatchFunction > rv(
        new BothBoundsWatcher<BoundType,ValueType>( min, max ) );
    value.change_is_OK = rv.get();
    return rv;
}

inline std::auto_ptr< Attribute<std::string>::ChangeWatchFunction >
create_bounds_watcher( 
    const Attribute< boost::optional<std::string> >& min, 
    Attribute<std::string>& value, 
    const Attribute< boost::optional<std::string> >& max 
) {
    return std::auto_ptr< Attribute<std::string>::ChangeWatchFunction >();
}

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
    range_checker = create_bounds_watcher( min, value, max );
}

template <typename TypeOfEntry>
Entry<TypeOfEntry>::Entry(
    string name, const TypeOfEntry& startVal)
: BasicEntry(name), 
  value( "value", startVal ),
  increment("increment", startVal ),
  min("min", bound_type() ),
  max("max", bound_type() )
{
    add_attributes( TypeOfEntry(), additional_attributes );
    range_checker = create_bounds_watcher( min, value, max );
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
    range_checker = create_bounds_watcher( min, value, max );
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
