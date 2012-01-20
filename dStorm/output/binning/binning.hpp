#ifndef DSTORM_OUTPUT_BINNING_BINNING_HPP
#define DSTORM_OUTPUT_BINNING_BINNING_HPP

#include "binning.h"

namespace dStorm {
namespace output {
namespace binning {

template <typename Implementation, typename Interface>
struct BinningAdapter
: public Interface
{
    BinningAdapter( const Implementation& i ) : i_(i) {}

    Interface* clone() const { return new BinningAdapter(*this); }
    void announce(const Output::Announcement& a) { i_.announce(a); }
    traits::ImageResolution resolution() const { return i_.resolution(); }
    int bin_points( const output::LocalizedImage& l, float* target, int stride ) const
        { return i_.bin_points( l, target, stride ); }
    boost::optional<float> bin_point( const dStorm::Localization& l ) const 
        { return i_.bin_point(l); }
    int field_number() const { return i_.field_number(); }

    float get_size() const { return i_.get_size(); }
    std::pair< float, float > get_minmax() const { return i_.get_minmax(); }
    double reverse_mapping( float v ) const { return i_.reverse_mapping(v); }
    void set_clipping( bool discard_outliers ) { i_.set_clipping( discard_outliers ); }

    void set_user_limit( bool lower_limit, const std::string& s ) 
        { i_.set_user_limit( lower_limit, s ); }
    bool is_bounded() const { return i_.is_bounded(); }
    display::KeyDeclaration key_declaration() const 
        { return i_.key_declaration(); }

  private:
    Implementation i_;
};

template <typename Interface, class Implementation>
std::auto_ptr< Interface > 
make_BinningAdapter( const Implementation& o ) {
    return std::auto_ptr<Interface>( 
        new BinningAdapter< Implementation, Interface >(o) );
}

}
}
}

#endif
