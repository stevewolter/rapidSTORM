#include "config.h"
#include "localization_config.h"

namespace dStorm {
namespace binning {

FieldChoice::FieldChoice( const std::string& name, const std::string& desc, BinningType type, std::string axis )
: simparm::ManagedChoiceEntry<FieldConfig>(name, desc) 
{
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
    if ( type == ScaledToInterval || type == InteractivelyScaledToInterval )
        addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis, 1.0f)) );
    else if ( (Traits::has_range && type == ScaledByResolution) )
        addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis, true) ));
    else if ( type == IsUnscaled )
        addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Field>(axis, false) ));
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
