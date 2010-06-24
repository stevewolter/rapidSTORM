#ifndef RESIDUEANALYSIS_IMPL_H
#define RESIDUEANALYSIS_IMPL_H
#include "main.h"
#include "no_analysis/main.h"
#include <fit++/FitFunction_impl.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/Input.h>
#include "Config.h"

template <typename Ty>
inline Ty sq(const Ty& a) { return a*a; }

namespace dStorm {
namespace gauss_2d_fitter {
namespace residue_analysis {

using namespace fitpp::Exponential2D;

template <int FF>
CommonInfo<FF>::CommonInfo(
    const Config& config, 
    const engine::JobInfo& info
) 
: no_analysis::CommonInfo<FF>(config, info),
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
CommonInfo<FF>::CommonInfo( const CommonInfo& o )
: no_analysis::CommonInfo<FF>(o),
  constants(o.constants),
  fit_function(o.fit_function),
  params( NULL, &constants),
  asymmetry_threshold(o.asymmetry_threshold),
  required_peak_distance_sq(o.required_peak_distance_sq) 
{}

template <int FF>
void CommonInfo<FF>::
start_from_splitted_single_fit(
    typename FitGroup::Variables* v,
    const Eigen::Vector2i& dir
)
{
    typedef no_analysis::CommonInfo<FF> Base;

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
CommonInfo<FF>::peak_distance_small( 
    typename FitGroup::Variables *variables
) {
    params.change_variable_set( variables );
    
    Eigen::Vector2d peak_dist =
        (params.template getPosition<0>() - params.template getPosition<1>());
    return ( peak_dist.squaredNorm() < required_peak_distance_sq );
}

template <int FF>
void
CommonInfo<FF>::set_two_kernel_improvement( 
    Localization& l,  float value )
{
    l.two_kernel_improvement() = value;
}

template <int FF>
bool CommonInfo<FF>::check_result( 
    typename FitGroup::Variables *, Localization *)
{
    std::stringstream error;
    error << "Unexpected call at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

template <int FF>
void CommonInfo<FF>::set_start(
    const engine::Spot& spot, const engine::BaseImage& image,
    double shift_estimate, typename FitGroup::Variables* variables
) {
    std::stringstream error;
    error << "Unexpected call at " << __FILE__ << ":" << __LINE__;
    throw std::logic_error(error.str());
}

}
}
}

#endif
