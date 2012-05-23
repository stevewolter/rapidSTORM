#ifndef DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_H
#define DSTORM_OUTPUT_BINNING_LOCALIZATION_CONFIG_H

#include "config.h"
#include "localization.h"
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <simparm/BoostUnits.hh>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace output {
namespace binning {

template <int Field>
class LocalizationConfig : public FieldConfig
{
    typedef typename Localization<Field,false>::TraitsType Traits;
    int row, column;
    static std::string make_ident(int r, int c);
    static std::string make_desc(int r, int c);

    typedef quantity< 
        typename divide_typeof_helper<typename Traits::OutputType::unit_type, camera::length>::type,
        typename Traits::OutputType::value_type>
        Resolution;
    simparm::Entry<Resolution> resolution;
    boost::optional<float> range;

    dStorm::default_on_copy< boost::signals2::signal<void()> > change;
    simparm::BaseAttribute::ConnectionStore listening;

    void attach_ui( simparm::NodeHandle at ) {
        simparm::NodeHandle r = attach_parent(at);
        if ( ! range.is_initialized() ) 
            resolution.attach_ui(r);
        listening = resolution.value.notify_on_value_change( change );
    }

  public:
    LocalizationConfig(std::string axis, int row, int column) ;
    LocalizationConfig(std::string axis, float range, int row, int column) ;

    LocalizationConfig* clone() const { return new LocalizationConfig(*this); }

    std::auto_ptr<Scaled> make_scaled_binner() const ;
    std::auto_ptr<Unscaled> make_unscaled_binner() const;
    virtual std::auto_ptr<UserScaled> make_user_scaled_binner() const;

    void set_visibility(const input::Traits<dStorm::Localization>& t, bool unscaled_suffices); 
    void add_listener( simparm::BaseAttribute::Listener& l );
};

}
}
}

#endif
