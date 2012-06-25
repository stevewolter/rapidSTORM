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
    int bin_points( const output::LocalizedImage& l, float* target, int stride ) const {
        int c = base->bin_points(l, target, stride);
        for (int i = 0; i < c; ++i)
            target[i*stride] = invert( target[i*stride] );
        return c;
    }
    boost::optional<float> bin_point( const Localization& l ) const {
        boost::optional<float> v = base->bin_point(l);
        if ( v.is_initialized() )
            return invert( *v );
        else    
            return v;
    }
    boost::optional<float> get_uncertainty( const Localization& l ) const {
        boost::optional<float> v = base->bin_point(l);
        if ( v.is_initialized() )
            return invert( *v );
        else    
            return v;
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
    void set_clipping( bool b ) { base->set_clipping(b); }
};

}
}
}

#endif
