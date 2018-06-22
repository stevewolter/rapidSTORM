#ifndef LOCPREC_PIXELATED_BESSEL_H
#define LOCPREC_PIXELATED_BESSEL_H

#include "traits/optics.h"
#include "engine/InputPlane.h"
#include <boost/units/Eigen/Core>
#include "types/samplepos.h"
#include "traits/Projection.h"

namespace input_simulation {

using namespace boost::units;

/** Sample space is measured in meters. */
class BesselFunction {
    /** Affine transformation from image space into sample space */
    const dStorm::engine::InputPlane& trafo;
    /** Position of fluorophore in sample space */
    const dStorm::samplepos fluorophore;
    const double na, n, theta_max;
    const quantity<si::length> lambda;
    const quantity< si::area > pixel_size;

  public:
    typedef dStorm::traits::Projection::SubpixelImagePosition Subpixel;

  private:
    struct IntegrationInfo;

    std::auto_ptr<IntegrationInfo> int_info;

    static double wavelength_callback( double, void * );
    static double line_integration_callback( double, void * );
    static double point_callback( double, void * );
    template <bool ImagPart>
    static double theta_callback( double, void * );
    double compute_point( const Subpixel& position, IntegrationInfo& ) const;

  public:
    typedef Eigen::Matrix< quantity<si::length,float>, 2, 1> HalfWidths;
    BesselFunction( 
        const dStorm::engine::InputPlane& transformation_into_sample_space,
        const dStorm::samplepos& fluorophore_position_in_sample_space,
        double num_apert, double opt_density,
        quantity<si::length> wavelength,
        quantity<si::area> pixel_size );

    double integrate( const Subpixel& at_pixel ) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

 }
 
#endif
