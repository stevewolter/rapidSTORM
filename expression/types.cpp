#include <stdexcept>
#include "types.h"
#include <boost/fusion/include/at_c.hpp>

namespace dStorm {
namespace expression {

std::string DynamicUnit::unit_names[BaseUnits::Number] = { 
    "px", "m", "fr", "s", "ADC"
};

bool approx_equal( double a, double b ) { return a >= 0.99999 * b && a <= 1.00001 * b; }

bool operator==( const DynamicUnit& a, const DynamicUnit& b )
{
    return a.isApprox( b, 1E-5 );
}

bool operator!=( const DynamicUnit& a, const DynamicUnit& b )
{
    return ! ( a == b );
}

bool operator==( const DynamicQuantity& a, const DynamicQuantity& b )
{
    if ( fsn::at_c<1>(a) != fsn::at_c<1>(b) ) {
        std::stringstream e;
        e << "Cannot compare quantities with units " << a.unit << " and " << b.unit;
        throw std::runtime_error(e.str());
    } else
        return approx_equal(fsn::at_c<0>(a), fsn::at_c<0>(b));
}

bool operator!=( const DynamicQuantity& a, const DynamicQuantity& b )
{
    return ! ( a == b );
}

#define ADAPT_OPERATOR(x,y) \
bool y( const DynamicQuantity& a, const DynamicQuantity& b ) \
{ \
    if ( fsn::at_c<1>(a) != fsn::at_c<1>(b) ) { \
        std::stringstream e; \
        e << "Cannot compare quantities with units " << a.unit << " and " << b.unit; \
        throw std::runtime_error(e.str()); \
    } \
    else \
        return fsn::at_c<0>(a) x fsn::at_c<0>(b); \
}
ADAPT_OPERATOR(>=, operator>=)
ADAPT_OPERATOR(<=, operator<=)
ADAPT_OPERATOR(>, operator>)
ADAPT_OPERATOR(<, operator<)

DynamicQuantity pow( const DynamicQuantity& a, const DynamicQuantity& b )
{
    if ( fsn::at_c<1>(b) != DynamicUnit(DynamicUnit::Zero()) )
        throw std::runtime_error("Exponent is not a dimensionless quantity");
    return DynamicQuantity( 
        std::pow( fsn::at_c<0>(a), fsn::at_c<0>(b) ),
        DynamicUnit(fsn::at_c<1>(a) * fsn::at_c<0>(b)) );
}

DynamicQuantity operator+( const DynamicQuantity& a, const DynamicQuantity& b )
{
    if ( fsn::at_c<1>(a) != fsn::at_c<1>(b) ) {
        std::stringstream e;
        e << "Cannot add quantities with units " << a.unit << " and " << b.unit;
        throw std::runtime_error(e.str());
    }
    return DynamicQuantity( fsn::at_c<0>(a) + fsn::at_c<0>(b), fsn::at_c<1>(a) );
}

DynamicQuantity operator-( const DynamicQuantity& a, const DynamicQuantity& b )
{
    if ( fsn::at_c<1>(a) != fsn::at_c<1>(b) ) {
        std::stringstream e;
        e << "Cannot subtract quantities with units " << a.unit << " and " << b.unit;
        throw std::runtime_error(e.str());
    }
    return DynamicQuantity( fsn::at_c<0>(a) - fsn::at_c<0>(b), fsn::at_c<1>(a) );
}

DynamicQuantity operator*( const DynamicQuantity& a, const DynamicQuantity& b )
{
    return DynamicQuantity( 
        fsn::at_c<0>(a) * fsn::at_c<0>(b),
        DynamicUnit(fsn::at_c<1>(a) + fsn::at_c<1>(b)) );
}

DynamicQuantity operator/( const DynamicQuantity& a, const DynamicQuantity& b )
{
    return DynamicQuantity( 
        fsn::at_c<0>(a) / fsn::at_c<0>(b),
        DynamicUnit(fsn::at_c<1>(a) - fsn::at_c<1>(b)) );
}

std::ostream& operator<<( std::ostream& o, const variable_table_index v )
{
    return o << "variable:" << int(v);
}

std::ostream& operator<<( std::ostream& o, const DynamicQuantity& v )
{
    return ( o << "(" << v.value << " " << v.unit << ")" );
}

std::ostream& operator<<( std::ostream& o, const DynamicUnit& v )
{
    bool first = true;
    for ( int i = 0; i < v.rows(); ++i ) {
        if ( abs(v[i]) <= 1E-6 ) continue;
        if ( !first ) o << " "; else first = false;
        o << DynamicUnit::unit_names[i];
        if ( abs(v[i]-1) > 1E-6 ) o << "^" << v[i];
    }
    return o;
}

}
}
