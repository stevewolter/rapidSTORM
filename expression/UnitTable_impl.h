#ifndef DSTORM_EXPRESSION_UNITTABLE_IMPL_H
#define DSTORM_EXPRESSION_UNITTABLE_IMPL_H

#include "expression/UnitTable.h"
#include <vector>
#include <boost/spirit/home/phoenix/function/function.hpp>

namespace dStorm {
namespace expression {

namespace fsn = boost::fusion;
namespace phx = boost::phoenix;

template <typename Type>
std::ostream& operator<<( std::ostream& o, const std::vector<Type>& v ) {
    for ( typename std::vector<Type>::const_iterator i = v.begin(); i != v.end(); ++i )
        o << *i;
    return o;
}

struct pow {
    template <typename Arg, typename Arg2>
    struct result { typedef Arg type; };

    template <typename Arg, typename Arg2>
    Arg operator()(Arg a, Arg2 b) const {
        return std::pow(a,b); 
    }
};

struct count_unit {
    template <typename Arg1, typename Arg2, typename Arg3>
    struct result { typedef Arg1 type; };

    DynamicUnit& table;
    count_unit( DynamicUnit& table ) : table(table) {}
    template <typename Arg1, typename Arg2, typename Arg3>
    Arg1 operator()(Arg1 prefix, Arg2 index, Arg3 exponent) const { 
        table[index] = table[index] + double(exponent);
        return std::pow(prefix, exponent);
    }
};

template <typename Iterator>
boost_units_unit_parser<Iterator>::boost_units_unit_parser()
: boost_units_unit_parser::base_type(unitspecs)
{
    using qi::labels::_val;
    using qi::labels::_1;
    using qi::labels::_2;
    using qi::labels::_3;

    exponent = ("^" >> qi::double_)[_val = _1] | qi::eps[_val = 1.0];
    si_prefix = prefixes[_val = _1];
    unitspec = (table >> exponent)[
            _val = phx::function<count_unit>(result)(phx::val(1.0), _1, _2) ]
            | (si_prefix >> table >> exponent)[
            _val = phx::function<count_unit>(result)(_1, _2, _3) ];
    unitspecs = qi::lit( "dimensionless" )[_val=1] | qi::eps[_val=1] >> +(unitspec[_val *= _1] );
}

}
}

#endif
