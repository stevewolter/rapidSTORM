#ifndef DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_IMPL_H
#define DSTORM_EXPRESSION_LOCALIZATION_VARIABLE_IMPL_H

#include "localization_variable.h"
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include "UnitTable.h"
#include <boost/units/io.hpp>
#include <dStorm/traits/tags.h>

namespace dStorm {
namespace expression {

template <typename Type>
struct optional_removed {
    typedef Type type; 
};
template <typename Type>
struct optional_removed< boost::optional<Type> > { typedef Type type; };

template <typename Type>
Type& deref_optional( Type& t ) { return t; }
template <typename Type>
const Type& deref_optional( const Type& t ) { return t; }
template <typename Type>
const Type& deref_optional( const boost::optional<Type>& t ) { return *t; }
template <typename Type>
Type& deref_optional( boost::optional<Type>& t ) { return *t; }

template <int Field, typename Tag>
Variable<Field,Tag>::Variable(const Scalar& s )
: variable( name( s ) ), scalar(s)
{
    std::string symbol = boost::units::symbol_string( typename Scalar::value_type::unit_type() );
    boost_units_unit_parser< std::string::const_iterator > parser;
    std::string::const_iterator begin = symbol.begin(), end = symbol.end();

    double result;

    bool success = boost::spirit::qi::phrase_parse(begin, end, parser, boost::spirit::ascii::space, result);
    assert( success && begin == end );

    unit = parser.result;
    scale = result;
}

template <int Field, typename Tag>
bool Variable<Field,Tag>::is_static( const input::Traits<Localization>& traits ) const
{
    return ! ( TaggedTraits::in_localization );
}

template <int Field, typename Tag>
DynamicQuantity Variable<Field,Tag>::get( const input::Traits<Localization>& traits ) const
{
    if ( TaggedTraits::in_traits ) {
        double rv = scale * deref_optional(scalar.template get<Tag>( traits )).value();
        return DynamicQuantity(rv, unit);
    } else {
        if ( Tag::is_given_tag::template in<TraitsType>::in_traits &&
             ! scalar.template get<typename Tag::is_given_tag>( traits ) )
            throw std::runtime_error("Tried to read variable " + this->variable::name + ", but it is not defined.");
        else
            return DynamicQuantity( std::numeric_limits<double>::quiet_NaN(), unit );
    }
}

template <int Field, typename Tag>
DynamicQuantity Variable<Field,Tag>::get( const Localization& l ) const
{
    if ( TaggedTraits::in_localization ) {
        double rv = scale * deref_optional(scalar.template get_field<Tag,Field>( l )).value();
        return DynamicQuantity(rv, unit);
    } else {
        throw std::logic_error("The variable " + this->variable::name + " is not defined in a localization");
    }
}

template <int Field, typename Tag>
void Variable<Field,Tag>::set( input::Traits<Localization>& traits, const DynamicQuantity& to ) const
{
    if ( TaggedTraits::in_traits ) {
        if ( unit != boost::fusion::at_c<1>(to) ) {
            std::stringstream e;
            e << "The variable " << variable::name << " has dimension " << unit << 
                 ", but is assigned from a quantity with dimension " << to.unit;
            throw std::runtime_error(e.str());
        }
        scalar.template set<Tag>( traits ) 
            = optional_removed<typename Scalar::template result_of<Tag>::set>::type::from_value( 
                boost::fusion::at_c<0>(to) / scale );
    } else if ( Tag::is_given_tag::template in<TraitsType>::in_traits ) {
        scalar.template set<typename Tag::is_given_tag>( traits ) = true;
    }
}

template <int Field, typename Tag, typename CheckedTag>
struct out_of_range {
    typedef typename Localization::Traits<Field>::type Traits;
    out_of_range( const traits::Scalar<Traits>&, const Traits& ) {}
    bool operator()( const Localization& ) const { return false; }
};

template <int Field>
struct out_of_range<Field, traits::min_tag, traits::value_tag> {
    typedef typename Localization::Traits<Field>::type Traits;
    const traits::Scalar<Traits>& scalar; const Traits& traits;
    out_of_range( const traits::Scalar<Traits>& s, const Traits& t ) : scalar(s), traits(t) {}
    bool operator()( const Localization& l ) const { 
        return ( scalar.range(traits).first.is_initialized() && *scalar.range( traits ).first > scalar.template get_field<traits::value_tag, Field>( l ) ); 
    }
};

template <int Field>
struct out_of_range<Field, traits::max_tag, traits::value_tag> {
    typedef typename Localization::Traits<Field>::type Traits;
    const traits::Scalar<Traits>& scalar; const Traits& traits;
    out_of_range( const traits::Scalar<Traits>& s, const Traits& t ) : scalar(s), traits(t) {}
    bool operator()( const Localization& l ) const {
        return ( scalar.range(traits).second.is_initialized() && *scalar.range( traits ).second < scalar.template get_field<traits::value_tag, Field>( l ) ); 
    }
};


template <int Field, typename Tag>
bool Variable<Field,Tag>::set( const input::Traits<Localization>& context,
    Localization& l, const DynamicQuantity& to ) const
{
    bool good = true;
    if ( TaggedTraits::in_localization ) {
        if ( unit != to.unit ) {
            std::stringstream e;
            e << "The variable " << variable::name << " has dimension " << unit << 
                 ", but is assigned from a quantity with dimension " << to.unit;
            throw std::runtime_error(e.str());
        }
        scalar.template set_field<Tag,Field>( l ) 
            = optional_removed<typename Scalar::template result_of<Tag>::set>::type::from_value( 
                boost::fusion::at_c<0>(to) / scale );
    }
    good = good && ! out_of_range<Field, traits::min_tag, Tag>( scalar, context )(l);
    good = good && ! out_of_range<Field, traits::max_tag, Tag>( scalar, context )(l);
    good = good && ! out_of_range<Field, Tag, traits::value_tag>( scalar, context )(l);
    return good;
}

template <int Field, typename Tag>
std::string Variable<Field,Tag>::name(const Scalar& s) 
{
    return s.template shorthand< Tag >();
}

}
}

#endif
