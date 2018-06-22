#ifndef DSTORM_GUF_OPTICS_H
#define DSTORM_GUF_OPTICS_H

#include "fit_window/Spot.h"
#include "traits/Projection.h"
#include "engine/Image_decl.h"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace engine { class InputPlane; }
namespace traits { class Optics; }
namespace fit_window {

/** This class converts detector A/D counts into photons. */
class Detector {
public:
    Detector( const traits::Optics& );
    bool can_compute_localization_precision() const { return has_precision; }
    float background_noise_variance() const { return *background_noise_variance_; }
    bool background_is_poisson_distributed() const 
        { return poisson_background_; }
    quantity< camera::intensity > photon_response() const { return photon_response_; }
    
    double absolute_in_photons( quantity<camera::intensity> ) const;
    double relative_in_photons( quantity<camera::intensity> ) const;

private:
    const quantity< camera::intensity > photon_response_;
    const quantity< camera::intensity, int > dark_current;
    const boost::optional< float > background_noise_variance_;
    const bool has_precision, poisson_background_;
};

/** This class encapsulates the back-transformation from an image to 
 *  sample space. */
class Optics
: public Detector
{
public:
    Optics( const Spot& max_distance, const engine::InputPlane& image_optics );
    bool supports_guaranteed_row_width() const;
    int get_fit_window_width(const Spot& at) const;

    quantity< si::area > pixel_size( const Spot& close_to ) const;
    typedef boost::optional< boost::units::quantity<boost::units::camera::length,int> >
        OptPixel;
    traits::Projection::ROI get_region_of_interest( const Spot& close_to, OptPixel width ) const;

private:
    const traits::Projection& projection;
    const Spot max_distance;
};

}
}

#endif
