#include "UnitTable_impl.h"
#include <string>
#include <boost/units/systems/si/area.hpp>
#include "dejagnu.h"

namespace dStorm {
namespace expression {

UnitTable::UnitTable()
{
    for (int i = 0; i < DynamicUnit().rows(); ++i)
        add( DynamicUnit::unit_names[i], i );
}

template class boost_units_unit_parser< std::string::const_iterator >;

void check_unit_parser( TestState& state ) {
    std::string in = "m m^2 m nm^2 pm^-1 px px^-3 px^4";
    boost_units_unit_parser< std::string::const_iterator > p;
    std::string::const_iterator begin = in.begin(), end = in.end();
    double result;

    DynamicUnit correct;
    correct[ *p.table.find("px") ] = 2;
    correct[ *p.table.find("m") ] = 5;

    bool r = phrase_parse(begin, end, p, boost::spirit::ascii::space, result);
    bool right_scale_factor = r && result >= 0.99E-6 && result <= 1.01E-6 ;
    bool units_right = r && p.result == correct;
    bool all_right = right_scale_factor && units_right;
    state( all_right && begin == end );
}

}
}
