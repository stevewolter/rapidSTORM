#ifndef DSTORM_OUTPUT_BINNING_INVERSION_H
#define DSTORM_OUTPUT_BINNING_INVERSION_H
#include "binning.h"
#include <dStorm/display/DataSource.h>

namespace dStorm {
namespace output {
namespace binning {

template <typename BaseType>
struct Inversion
: public BaseType
{
    boost::shared_ptr<BaseType> base;
    std::pair< float, float > range;

    float invert( float value ) const
        { return range.first + (range.second - value); }
  public:
    Inversion( boost::shared_ptr<BaseType> base ) : base(base) {}
    Inversion* clone() const { return new Inversion(*this); }

    void announce(const Output::Announcement& a) { base->announce(a); range = base->get_minmax(); }
    traits::ImageResolution resolution() const { return base->resolution(); }
    void bin_points( const output::LocalizedImage& l, float* target, int stride ) const {
        base->bin_points(l, target, stride);
        for (output::LocalizedImage::const_iterator i = l.begin(); i != l.end(); ++i) {
            *target = invert(*target);
            target += stride;
        }
    }
    float bin_point( const Localization& l ) const {
        return invert( base->bin_point(l) );
    }

    int field_number() const { return base->field_number(); }
    float get_size() const { return base->get_size(); }
    std::pair< float, float > get_minmax() const { return base->get_minmax(); }
    double reverse_mapping( float value ) const 
        { return base->reverse_mapping( invert( value ) ); }
    void set_user_limit( bool lower_limit, const std::string& s ) 
        { throw std::runtime_error("Setting a user-defined limit on inverted axes is not implemented. Sorry."); }
    bool is_bounded() const { return base->is_bounded(); }
    display::KeyDeclaration key_declaration() const { return base->key_declaration(); }
};

}
}
}

#endif
