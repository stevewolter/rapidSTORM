#include "config.h"
#include "localization_config.h"
#include <simparm/ChoiceEntry_Impl.h>

namespace dStorm {
namespace output {
namespace binning {

FieldChoice::FieldChoice( const std::string& name, const std::string& desc, BinningType type, std::string axis )
: simparm::ManagedChoiceEntry<FieldConfig>(name, desc) {
    fill<0>(type, axis);
}

FieldChoice::FieldChoice(const FieldChoice& o) 
: simparm::ManagedChoiceEntry<FieldConfig>(o)
{
}

void FieldChoice::set_visibility(const input::Traits<dStorm::Localization>& t, bool unscaled_suffices)
{
    for ( iterator i = begin(); i != end(); ++i )
        i->set_visibility( t, unscaled_suffices );
}

template <int Field> 
void FieldChoice::fill(BinningType type, std::string axis)
{
    typedef typename Localization<Field,false>::TraitsType Traits;
    for (int i = 0; i < Traits::Rows; ++i)
        for (int j = 0; j < Traits::Cols; ++j)
            if ( type == ScaledToInterval || type == InteractivelyScaledToInterval )
                addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis,1.0, i, j)) );
            else if ( (Traits::has_range && type == ScaledByResolution) || type == IsUnscaled )
                addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis,i, j)) );
    fill<Field+1>(type, axis);
}

template <>
void FieldChoice::fill< dStorm::Localization::Fields::Count >(BinningType type, std::string axis)
{}

FieldChoice::~FieldChoice() {}

void FieldChoice::add_listener( simparm::BaseAttribute::Listener l ) {
    change.connect( l );
    for ( iterator i = begin(); i != end(); ++i )
        i->add_listener( l );
}

void FieldChoice::attach_ui( simparm::NodeHandle a ) {
    simparm::ManagedChoiceEntry<FieldConfig>::attach_ui(a);
    listening = value.notify_on_value_change( change );
}

}
}
}
