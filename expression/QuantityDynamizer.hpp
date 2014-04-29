#ifndef DSTORM_EXPRESSION_QUANTITY_DYNAMIZER_HPP
#define DSTORM_EXPRESSION_QUANTITY_DYNAMIZER_HPP

#include "expression/QuantityDynamizer.h"
#include "expression/UnitTable.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace expression {

template <typename Quantity>
QuantityDynamizer<Quantity>::QuantityDynamizer()
{
    std::string symbol = boost::units::symbol_string( typename Quantity::unit_type() );
    boost_units_unit_parser< std::string::const_iterator > parser;
    std::string::const_iterator begin = symbol.begin(), end = symbol.end();

    double result;

    bool success = boost::spirit::qi::phrase_parse(begin, end, parser, boost::spirit::ascii::space, result);
    if ( !success || begin != end ) {
        throw std::logic_error("Unable to parse quantity string " + symbol);
    }

    unit = parser.result;
    scale = result;
}

template <typename Quantity>
Quantity 
QuantityDynamizer<Quantity>::operator()( const DynamicQuantity& v ) const
{
    if ( unit != boost::fusion::at_c<1>(v) ) {
        std::stringstream e;
        e << "Cannot assign a ";
        if ( v.unit == DynamicUnit::Dimensionless() )
            e << "dimensionless quantity";
        else
            e << "quantity with dimension " << v.unit;
        e << " to a ";
        if ( unit == DynamicUnit::Dimensionless() )
            e << "dimensionless variable";
        else
            e << "variable with dimension " << unit;
        throw std::runtime_error(e.str());
    }
    return Quantity::from_value( static_cast<typename Quantity::value_type>(
            boost::fusion::at_c<0>(v) / scale ) );
}

template <typename Quantity>
DynamicQuantity 
QuantityDynamizer<Quantity>::operator()( const Quantity& q ) const
{
    return DynamicQuantity(scale * q.value(), unit);
}

}
}

#endif
