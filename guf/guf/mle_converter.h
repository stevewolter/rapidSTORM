#ifndef DSTORM_GUF_MLE_CONVERTER_H
#define DSTORM_GUF_MLE_CONVERTER_H

#include <dStorm/traits/optics.h>
#include <dStorm/engine/Image.h>

namespace dStorm {
namespace guf {

struct mle_converter {
    dStorm::engine::StormPixel dark_current;
    double photon_response;
  public:
    typedef double result_type;
    mle_converter( 
        quantity<camera::intensity,int> dark,
        quantity<camera::intensity,double> photon_response
    )
        : dark_current( dark.value() ),
          photon_response( photon_response.value() ) {}
    mle_converter( const dStorm::traits::Optics<2>& input )
        : dark_current( input.dark_current.get_value_or( 0*camera::ad_count ).value() ),
          photon_response( input.photon_response.get_value_or( 1 * camera::ad_count).value() ) {}
    double operator() ( const dStorm::engine::StormPixel& pixel ) const
        { return std::max( (pixel - dark_current) / photon_response , 0.0 ); }

    quantity<si::dimensionless> convert_amplitude( quantity<camera::intensity> amp ) const
        { return amp.value() / photon_response; }
    quantity<si::dimensionless> convert_shift( quantity<camera::intensity> shift ) const
        { return ( shift.value() - dark_current ) / photon_response; }
};

}
}

#endif
