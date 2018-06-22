#include "binning/config.h"

#include <boost/mpl/for_each.hpp>

#include "binning/localization_config_impl.h"
#include "localization/Fields.h"

namespace dStorm {
namespace binning {

struct FieldChoiceAdder {
    typedef void result_type;
    template <typename Tag> 
    void operator()(BinningType type, std::string axis, simparm::ManagedChoiceEntry<FieldConfig>& choice, Tag tag)
    {
        typedef localization::MetaInfo<Tag> Traits;
        if ( type == ScaledToInterval || type == InteractivelyScaledToInterval )
            choice.addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Tag>(axis, 1.0f)) );
        else if ( (Traits::has_range && type == ScaledByResolution) )
            choice.addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Tag>(axis, true) ));
        else if ( type == IsUnscaled )
            choice.addChoice( std::auto_ptr<FieldConfig>(new LocalizationConfig<Tag>(axis, false) ));
    }
};

FieldChoice::FieldChoice( const std::string& name, const std::string& desc, BinningType type, std::string axis )
: simparm::ManagedChoiceEntry<FieldConfig>(name, desc) 
{
    boost::mpl::for_each<localization::Fields>(boost::bind(
        FieldChoiceAdder(),
        type,
        axis,
        boost::ref(static_cast<simparm::ManagedChoiceEntry<FieldConfig>&>(*this)),
        _1));
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
