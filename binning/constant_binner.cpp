#include "binning/constant_binner.h"
#include "binning/binning.h"

namespace dStorm {
namespace binning {

struct ConstantBinner 
: public Unscaled
{
    ~ConstantBinner() {}
    ConstantBinner* clone() const { return new ConstantBinner(); }
    void announce(const output::Output::Announcement&) {}
    traits::ImageResolution resolution() const 
        { throw std::logic_error("Constant binners have no resolution"); }
    int bin_points( const output::LocalizedImage& er, float* target, int stride ) const {
        for ( output::LocalizedImage::const_iterator i = er.begin(); i != er.end(); ++i )
        {
            *target = 1;
            target += stride;
        }
        return er.size();
    }
    boost::optional<float> bin_point( const Localization& ) const 
        { return 1.0f; }
    boost::optional<float> get_uncertainty( const Localization& ) const 
        { return 0.0f; }

    int field_number() const { throw std::logic_error("No associated "
        "localization field for constant binner"); }
};

class ConstantFieldConfig : public FieldConfig {
    ~ConstantFieldConfig() {}
    ConstantFieldConfig* clone() const { return new ConstantFieldConfig(); }

    std::auto_ptr<Scaled> make_scaled_binner() const 
        { throw std::logic_error("Constant binner cannot be scaled"); }
    std::auto_ptr<Unscaled> make_unscaled_binner() const
        { return make_constant_binner(); }
    std::auto_ptr<UserScaled> make_user_scaled_binner() const
        { throw std::logic_error("Constant binner cannot be user-scaled"); }
    std::auto_ptr<Unscaled> make_uncertainty_binner() const
        { throw std::logic_error("Constant binner has no uncertainty"); }
    void set_visibility(const input::Traits<Localization>&, bool unscaled_suffices) {}

    void add_listener( simparm::BaseAttribute::Listener& ) {}
    void attach_ui( simparm::NodeHandle at ) { attach_parent(at); }
public:
    ConstantFieldConfig() : FieldConfig("Constant", "Localization count") {}
};


std::auto_ptr<Unscaled> make_constant_binner()
{
    return std::auto_ptr<Unscaled>( new ConstantBinner() );
}

std::auto_ptr<FieldConfig> make_constant_binner_config()
{
    return std::auto_ptr<FieldConfig>( new ConstantFieldConfig() );
}

}
}

