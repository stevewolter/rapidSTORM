#include "UnitTable.h"
#include "UnitTable_impl.h"
#include <boost/units/systems/si/area.hpp>

using namespace dStorm::expression;

int main(int argc, char *argv[]) {
    std::string in;
    if ( argc > 1 )
        in = argv[1];
    else 
        in = "m m^2 m nm^2 pm^-1 px px^-3 px^4";
    boost_units_unit_parser< std::string::const_iterator > p;
    std::string::const_iterator begin = in.begin(), end = in.end();
    double result;

    DynamicUnit correct;
    correct[ *p.table.find("px") ] = 7;
    correct[ *p.table.find("m") ] = 4;

    bool r = phrase_parse(begin, end, p, boost::spirit::ascii::space, result);
    bool right_scale_factor = r && result >= 0.99E-6 && result <= 1.01E-6 ;
    //bool units_right = r && p.result == correct;
    bool all_right = right_scale_factor;
    if ( all_right && begin == end )
        return 0;
    else {
        std::cerr << "Parsing stopped at character '" << *begin << "' (no. " << (begin - in.begin()) << ")" << std::endl;
        std::cerr << "Expected result 1E-6, got " << result << std::endl;
        return 1;
    }
}
