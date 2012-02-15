#ifndef LOCPREC_PIXELATED_BESSEL_H
#define LOCPREC_PIXELATED_BESSEL_H

#include <dStorm/traits/optics.h>
#include <boost/units/Eigen/Core>
#include <dStorm/types/samplepos.h>
#include <dStorm/traits/Projection.h>

namespace locprec {

using namespace boost::units;

/** Sample space is measured in meters. */
class BesselFunction {
    /** Affine transformation from image space into sample space */
    const dStorm::traits::Optics<2>& trafo;
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
        const dStorm::traits::Optics<2>& transformation_into_sample_space,
        const dStorm::samplepos& fluorophore_position_in_sample_space,
        double num_apert, double opt_density,
        quantity<si::length> wavelength,
        quantity<si::area> pixel_size );

    double integrate( const Subpixel& at_pixel ) const;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

 }
 
#endif
