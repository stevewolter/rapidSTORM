#include "localization_config_impl.h"

namespace dStorm {
namespace output {
namespace binning {

template <int Field>
std::auto_ptr<FieldConfig>
inline impl_make_localization_config(std::string axis, int field_index, int row, int column)
{
    if ( field_index == Field )
        return std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis, row, column));
    else
        return impl_make_localization_config<Field+1>(axis, field_index, row, column);
}

template <>
std::auto_ptr<FieldConfig>
inline impl_make_localization_config<dStorm::Localization::Fields::Count>
    (std::string, int field_index, int, int)
{
    std::stringstream error;
    error << "Config for localization field no. " << field_index << 
             " requested, which does not exist.";
    throw std::logic_error(error.str());
}

std::auto_ptr<FieldConfig> make_localization_config(std::string axis, int field_index, int row, int column)
{
    return impl_make_localization_config<0>(axis, field_index, row, column);
}


#define INSTANTIATE_WITH_LOCALIZATION_FIELD_INDEX(x) \
    template class LocalizationConfig<x>;
#include <dStorm/localization/expand.h>

}
}
}
