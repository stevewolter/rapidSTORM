#include "GaussFitter_Width_Invariants.h"
#include <fit++/FitFunction_impl.hh>
#include <dStorm/engine/Image_impl.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/Input.h>
#include "GaussFitterConfig.h"

template <typename Ty>
Ty sq(const Ty& a) { return a*a; }

namespace dStorm {
namespace engine {

template <int FF>
Width_Invariants<FF,false>::Width_Invariants( 
   const GaussFitterConfig& c, const JobInfo& info
) 
: params( NULL, &constants),
  amplitude_threshold( info.config.amplitude_threshold() ),
  start_sx( info.config.sigma_x() ),
  start_sy( info.config.sigma_y() ),
  start_sxy( info.config.sigma_xy() )
{
    fit_function.
        setStartLambda( c.marquardtStartLambda() );
    fit_function.
        setMaximumIterationSteps( c.maximumIterationSteps() );
    fit_function.
        setSuccessiveNegligibleStepLimit( 
            c.successiveNegligibleSteps() );

    FitGroup::template set_absolute_epsilon<MeanX,0>
        (fit_function, c.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanY,0>
        (fit_function, c.negligibleStepLength());

    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) ) )
        params.template setSigmaX<0>( start_sx );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) ) )
        params.template setSigmaY<0>( start_sy );
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) ) )
        params.template setSigmaXY<0>( start_sxy );
}

template <int FF>
Width_Invariants<FF,true>::Width_Invariants( 
    const GaussFitterConfig& config, const JobInfo& info
) 
: Width_Invariants<FF,false>(config, info),
  params( NULL, &constants),
  asymmetry_threshold( config.asymmetry_threshold() ),
  required_peak_distance_sq( 
    sq( (config.required_peak_distance() * 1E-9 * boost::units::si::meter)
        * (*info.traits.resolution) / cs_units::camera::pixel ) )
{
    fit_function
        .setStartLambda( config.marquardtStartLambda() );
    fit_function.setMaximumIterationSteps(
        config.maximumIterationSteps() );
    fit_function.setSuccessiveNegligibleStepLimit( 
            config.successiveNegligibleSteps() );
    FitGroup::template set_absolute_epsilon<MeanX,0>
        (fit_function, config.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanY,0>
        (fit_function, config.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanX,1>
        (fit_function, config.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanY,1>
        (fit_function, config.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<Amplitude,0>
        (fit_function, 1);
    FitGroup::template set_absolute_epsilon<Amplitude,1>
        (fit_function, 1);

    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) ) ) {
        params.template setSigmaX<0>( this->start_sx );
        params.template setSigmaX<1>( this->start_sx );
    }
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) ) ) {
        params.template setSigmaY<0>( this->start_sy );
        params.template setSigmaY<1>( this->start_sy );
    }
    if ( ! ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) ) ) {
        params.template setSigmaXY<0>( this->start_sxy );
        params.template setSigmaXY<1>( this->start_sxy );
    }
}

template <int FF>
StartInformation
Width_Invariants<FF,false>::set_start(
    const Spot& spot, 
    const Image& image,
    double shift_estimate,
    typename FitGroup::Variables* variables 
) 
{
    params.change_variable_set( variables );
    params.template setMeanX<0>( spot.x() );
    params.template setMeanY<0>( spot.y() );
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) )
        params.template setSigmaX<0>(start_sx);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) )
        params.template setSigmaY<0>(start_sy);
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) )
        params.template setSigmaXY<0>(start_sxy);

    int xc = round(spot.x()), yc = round(spot.y());
    double center = image(xc,yc);
    
    params.setShift( shift_estimate );
    params.template setAmplitude<0>( 
        max<double>(center - shift_estimate, 10)
        * 2 * M_PI 
        * params.template getSigmaX<0>() * params.template getSigmaY<0>());

    StartInformation si;
    si.maxs.x() = image.width-1 - 1;
    si.maxs.y() = image.height-1 - 1;
    si.start.x() = spot.x();
    si.start.y() = spot.y();
    return si;
}

template <int FF>
bool 
Width_Invariants<FF,false>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target,
    const StartInformation& start
)
{
    params.change_variable_set( variables );

    Localization::Position p;
    p.x() = params.template getMeanX<0>() * cs_units::camera::pixel;
    p.y() = params.template getMeanY<0>() * cs_units::camera::pixel;

    new(target) Localization( 
        p, float( params.template getAmplitude<0>() )
                * cs_units::camera::ad_counts );

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
        && target->x() < start.maxs.x() * cs_units::camera::pixel
        && target->y() < start.maxs.y() * cs_units::camera::pixel
        && sq(target->x().value() - start.start.x()) + 
           sq(target->y().value() - start.start.y()) < 4;
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

    return good;
}

template <int FF>
void Width_Invariants<FF,true>::
start_from_splitted_single_fit(
    typename FitGroup::Variables* v,
    const Eigen::Vector2i& dir
)
{
    typedef Width_Invariants<FF,false> Base;

    params.change_variable_set( v );
    params.setShift( Base::params.getShift() );

    float half_dist = 1.8;
    params.template setMeanX<0>
        ( Base::params.template getMeanX<0>() + dir.x()*half_dist );
    params.template setMeanX<1>
        ( Base::params.template getMeanX<0>() - dir.x()*half_dist );
    params.template setMeanY<0>
        ( Base::params.template getMeanY<0>() + dir.y()*half_dist );
    params.template setMeanY<1>
        ( Base::params.template getMeanY<0>() - dir.y()*half_dist );
    params.template setAmplitude<0>
        ( Base::params.template getAmplitude<0>() / 2 );
    params.template setAmplitude<1>
        ( Base::params.template getAmplitude<0>() / 2 );
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaX ) ) {
        params.template setSigmaX<0>( this->start_sx );
        params.template setSigmaX<1>( this->start_sx );
    }
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaY ) ) {
        params.template setSigmaY<0>( this->start_sy );
        params.template setSigmaY<1>( this->start_sy );
    }
    if ( FF & ( 1 << fitpp::Exponential2D::SigmaXY ) ) {
        params.template setSigmaXY<0>( this->start_sxy );
        params.template setSigmaXY<1>( this->start_sxy );
    }

}

template <int FF>
bool
Width_Invariants<FF, true>::peak_distance_small( 
    typename FitGroup::Variables *variables
) {
    params.change_variable_set( variables );
    
    Eigen::Vector2d peak_dist =
        (params.template getPosition<0>() - params.template getPosition<1>());
    return ( peak_dist.squaredNorm() < required_peak_distance_sq );
}

template class Width_Invariants<fitpp::Exponential2D::FreeForm, false>;
template class Width_Invariants<fitpp::Exponential2D::FreeForm, true>;
template class Width_Invariants<fitpp::Exponential2D::FreeForm_NoCorrelation, false>;
template class Width_Invariants<fitpp::Exponential2D::FreeForm_NoCorrelation, true>;
template class Width_Invariants<fitpp::Exponential2D::FixedForm, false>;
template class Width_Invariants<fitpp::Exponential2D::FixedForm, true>;

}
}
