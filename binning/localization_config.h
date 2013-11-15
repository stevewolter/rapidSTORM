#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_H

#include "config.h"
#include "localization.h"
#include <simparm/Object.h>
#include <simparm/Entry.h>
#include <simparm/BoostUnits.h>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace binning {

template <typename Tag>
class LocalizationConfig : public FieldConfig
{
    bool use_resolution;

    typedef quantity< 
        typename divide_typeof_helper<typename Tag::OutputType::unit_type, camera::length>::type,
        typename Tag::OutputType::value_type>
        Resolution;
    simparm::Entry<Resolution> resolution;
    boost::optional<float> range;

    dStorm::default_on_copy< boost::signals2::signal<void()> > change;
    simparm::BaseAttribute::ConnectionStore listening;

    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = attach_parent(at);
        if ( use_resolution ) resolution.attach_ui(r);
        listening = resolution.value.notify_on_value_change( change );
    }

  public:
    LocalizationConfig(std::string axis, bool use_resolution) ;
    LocalizationConfig(std::string axis, float range) ;

    LocalizationConfig* clone() const { return new LocalizationConfig(*this); }

    std::auto_ptr<Scaled> make_scaled_binner() const ;
    std::auto_ptr<Unscaled> make_unscaled_binner() const;
    std::auto_ptr<UserScaled> make_user_scaled_binner() const;
    std::auto_ptr<Unscaled> make_uncertainty_binner() const;

    void set_visibility(const input::Traits<dStorm::Localization>& t, bool unscaled_suffices); 
    void add_listener( simparm::BaseAttribute::Listener& l );
};

}
}

#endif
