#ifndef DEBUG
#include "debug.h"
#endif
#include "main.h"
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include "Config.h"
#include "fitter/MarquardtInfo_impl.h"

template <typename Ty>
inline Ty sqr(const Ty& a) { return a*a; }

namespace dStorm {
namespace gauss_2d_fitter {
namespace no_analysis {

using namespace fitpp::Exponential2D;
using dStorm::engine::Spot;
using dStorm::engine::BaseImage;

template <int Ks,int FF>
CommonInfo<Ks,FF>::CommonInfo( 
   const Config& c, const engine::JobInfo& info
) 
: fitter::MarquardtInfo<FitGroup::VarC>(c,info),
  amplitude_threshold( info.config.amplitude_threshold() / cs_units::camera::ad_counts ),
  start_sx( info.config.sigma_x() / cs_units::camera::pixel ),
  start_sy( info.config.sigma_y() / cs_units::camera::pixel ),
  start_sxy( info.config.sigma_xy() ),
  params( NULL, &constants)
{
    FitGroup::template Parameter<MeanX>::set_absolute_epsilon
        (this->fit_function, c.negligibleStepLength());
    FitGroup::template Parameter<MeanY>::set_absolute_epsilon
        (this->fit_function, c.negligibleStepLength());

    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) ) )
        params.template set_all_SigmaX( start_sx );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) ) )
        params.template set_all_SigmaY( start_sy );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) ) )
        params.template set_all_SigmaXY( start_sxy );
}

template <int Kernels, int FF>
CommonInfo<Kernels,FF>::CommonInfo( const CommonInfo& o ) 
: fitter::MarquardtInfo<FitGroup::VarC>(o),
  maxs(o.maxs), start(o.start), amplitude_threshold(o.amplitude_threshold),
  start_sx(o.start_sx), start_sy(o.start_sy), start_sxy(o.start_sxy),
  constants(o.constants),
  params( NULL, &constants )
{
}

template <int Kernels,int FF>
void
CommonInfo<Kernels,FF>::set_start(
    typename FitGroup::Variables* variables 
) 
{
    params.change_variable_set( variables );

    if ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) )
        params.template set_all_SigmaX(start_sx);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) )
        params.template set_all_SigmaY(start_sy);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) )
        params.template set_all_SigmaXY(start_sxy);
}

template <int Kernels,int FF>
void
CommonInfo<Kernels,FF>::set_start(
    const Spot& spot, 
    const BaseImage& image,
    double shift_estimate,
    typename FitGroup::Variables* variables 
) 
{
    set_start(variables);
    params.template set_all_MeanX( spot.x() );
    params.template set_all_MeanY( spot.y() );

    int xc = round(spot.x()), yc = round(spot.y());
    double center = image(xc,yc);
    
    params.setShift( shift_estimate );
    params.template set_all_Amplitude( 
        std::max<double>(center - shift_estimate, 10)
        * 2 * M_PI 
        * params.template getSigmaX<0>() * params.template getSigmaY<0>());

    DEBUG( "Estimating center at " << center << ", shift at " << shift_estimate 
              << " for spot at " << xc << " " << yc << " with image sized " << image.width_in_pixels() << " by "
              << image.height_in_pixels() );
    DEBUG( "Sigmas are " << params.template getSigmaX<0>() << " " << params.template getSigmaY<0>()
              );
    DEBUG( "Set start " << *variables );

    maxs.x() = image.width_in_pixels()-1 - 1;
    maxs.y() = image.height_in_pixels()-1 - 1;
    start.x() = spot.x();
    start.y() = spot.y();
}

template <int Kernels,int FF>
bool 
CommonInfo<Kernels,FF>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target
)
{
    params.change_variable_set( variables );

    Localization::Position p;
    p.x() = params.template getMeanX<0>() * cs_units::camera::pixel;
    p.y() = params.template getMeanY<0>() * cs_units::camera::pixel;

    new(target) Localization( 
        p, float( params.template getAmplitude<0>() )
                * cs_units::camera::ad_counts );

    DEBUG("Got fit " << p.x() << " " << p.y() << " " << target->strength());
    bool sx_correct = ( ! (FF & ( 1 << fitpp::Exponential2D::SigmaX )))
        || ( params.template getSigmaX<0>() >= start_sx/4
          && params.template getSigmaX<0>() <= start_sx*4 );
    bool sy_correct = ( ! (FF & ( 1 << fitpp::Exponential2D::SigmaY )))
        || ( params.template getSigmaX<0>() >= start_sy/4
          && params.template getSigmaX<0>() <= start_sy*4 );
    bool sigmas_correct = sx_correct && sy_correct;

    bool good = sigmas_correct
        && target->strength() > amplitude_threshold 
                                * cs_units::camera::ad_counts
        && target->x() >= 1*cs_units::camera::pixel
        && target->y() >= 1*cs_units::camera::pixel
        && target->x() < maxs.x() * cs_units::camera::pixel
        && target->y() < maxs.y() * cs_units::camera::pixel
        && sqr(target->x().value() - start.x()) + 
           sqr(target->y().value() - start.y()) < 4;
    if ( (FF != fitpp::Exponential2D::FixedForm) ) {
        double sx = params.template getSigmaX<0>(),
               sy = params.template getSigmaY<0>(),
               corr = params.template getSigmaXY<0>();
        target->fit_covariance_matrix()(0,0) 
            = sx*sx * cs_units::camera::pixel *
                      cs_units::camera::pixel;
        target->fit_covariance_matrix()(0,1)
            = target->fit_covariance_matrix()(1,0)
            = corr*sx*sy * cs_units::camera::pixel
                         * cs_units::camera::pixel;
        target->fit_covariance_matrix()(1,1) = sy*sy
            * cs_units::camera::pixel 
            * cs_units::camera::pixel;
    }

    target->unset_source_trace();
    return good;
}


}
}
}

