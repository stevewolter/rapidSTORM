#include "config.h"
#include "localization_config.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace output {
namespace binning {

FieldChoice::FieldChoice( const std::string& name, const std::string& desc, BinningType type, std::string axis )
: simparm::NodeChoiceEntry<FieldConfig>(name, desc) {
    fill<0>(type, axis);
}

void FieldChoice::set_visibility(const input::Traits<dStorm::Localization>& t, bool unscaled_suffices)
{
    for ( iterator i = beginChoices(); i != endChoices(); ++i ) {
        i->set_visibility( t, unscaled_suffices );
    }
}

template <int Field> 
void FieldChoice::fill(BinningType type, std::string axis)
{
    typedef typename Localization<Field,false>::TraitsType Traits;
    if ( type == ScaledToInterval || type == InteractivelyScaledToInterval )
        for (int i = 0; i < Traits::Rows; ++i)
            for (int j = 0; j < Traits::Cols; ++j)
                addChoice( new LocalizationConfig<Field>(axis,1.0, i, j) );
    else if ( (Traits::has_range && type == ScaledByResolution) || type == IsUnscaled )
        for (int i = 0; i < Traits::Rows; ++i)
            for (int j = 0; j < Traits::Cols; ++j)
                addChoice( new LocalizationConfig<Field>(axis,i, j) );
    fill<Field+1>(type, axis);
}

template <>
void FieldChoice::fill< dStorm::Localization::Fields::Count >(BinningType type, std::string axis)
{}

FieldChoice::FieldChoice(const FieldChoice& o) : simparm::NodeChoiceEntry<FieldConfig>(o, DeepCopy) {}
FieldChoice::~FieldChoice() {}


}
}
}
