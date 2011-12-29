#include "UnitTable_impl.h"
#include <string>

namespace dStorm {
namespace expression {

UnitTable::UnitTable()
{
    for (int i = 0; i < DynamicUnit().rows(); ++i)
        add( DynamicUnit::unit_names[i], i );
}

template class boost_units_unit_parser< std::string::const_iterator >;

}
}
