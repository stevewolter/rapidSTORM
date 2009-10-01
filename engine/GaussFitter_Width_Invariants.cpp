#include "engine/GaussFitter_Width_Invariants.h"
#include <fit++/FitFunction_impl.hh>
#include <CImg.h>

template <typename Ty>
Ty sq(const Ty& a) { return a*a; }

namespace dStorm {

template <bool FS>
Width_Invariants<FS,false>::Width_Invariants( 
    const Config& config 
) 
: params( NULL, &constants),
  amplitude_threshold( config.amplitude_threshold() ),
  start_sx( config.sigma_x() ),
  start_sy( config.sigma_y() ),
  start_sxy( config.sigma_xy() )
{
    fit_function.
        setStartLambda( config.marquardtStartLambda() );
    fit_function.
        setMaximumIterationSteps( config.maximumIterationSteps() );
    fit_function.
        setSuccessiveNegligibleStepLimit( 
            config.successiveNegligibleSteps() );

    FitGroup::template set_absolute_epsilon<MeanX,0>
        (fit_function, config.negligibleStepLength());
    FitGroup::template set_absolute_epsilon<MeanY,0>
        (fit_function, config.negligibleStepLength());

    if ( ! FS ) {
        params.template setSigmaX<0>( start_sx );
        params.template setSigmaY<0>( start_sy );
        params.template setSigmaXY<0>( start_sxy );
    }
}

template <bool FS>
Width_Invariants<FS,true>::Width_Invariants( 
    const Config& config 
) 
: Width_Invariants<FS,false>(config),
  params( NULL, &constants),
  asymmetry_threshold( config.asymmetry_threshold() ),
  residue_threshold( 0.98 )
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

    if ( ! FS ) {
        params.template setSigmaX<0>( this->start_sx );
        params.template setSigmaY<0>( this->start_sy );
        params.template setSigmaXY<0>( this->start_sxy );
        params.template setSigmaX<1>( this->start_sx );
        params.template setSigmaY<1>( this->start_sy );
        params.template setSigmaXY<1>( this->start_sxy );
    }
}

template <bool FS>
StartInformation
Width_Invariants<FS,false>::set_start(
    const Spot& spot, 
    const Image& image,
    double shift_estimate,
    typename FitGroup::Variables* variables 
) 
{
    params.change_variable_set( variables );
    params.template setMeanX<0>( spot.x() );
    params.template setMeanY<0>( spot.y() );
    if ( FS ) {
        params.template setSigmaX<0>(start_sx);
        params.template setSigmaY<0>(start_sy);
        params.template setSigmaXY<0>(start_sxy);
    }

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

template <bool FS>
bool 
Width_Invariants<FS,false>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target,
    const StartInformation& start
)
{
    params.change_variable_set( variables );

    new(target) 
        Localization( params.template getMeanX<0>(),
                      params.template getMeanY<0>(), 
                      -1, 
                      params.template getAmplitude<0>() );
    bool sigmas_correct = !FS
        && params.template getSigmaX<0>() >= start_sx/4
        && params.template getSigmaY<0>() >= start_sy/4 
        && params.template getSigmaX<0>() <= start_sx*4 
        && params.template getSigmaY<0>() <= start_sy*4;

    return sigmas_correct
        && target->getStrength() > amplitude_threshold 
        && target->x() >= 1 && target->y() >= 1 
        && target->x() < start.maxs.x() && target->y() < start.maxs.y()
        && sq(target->x() - start.start.x()) + 
           sq(target->y() - start.start.y()) < 4;
}

template <bool FS>
void Width_Invariants<FS,true>::
start_from_splitted_single_fit(
    typename FitGroup::Variables* v,
    const Eigen::Vector2i& dir
)
{
    typedef Width_Invariants<FS,false> Base;

    params.change_variable_set( v );
    params.setShift( Base::params.getShift() );

    params.template setMeanX<0>
        ( Base::params.template getMeanX<0>() + dir.x()*2 );
    params.template setMeanX<1>
        ( Base::params.template getMeanX<0>() - dir.x()*2 );
    params.template setMeanY<0>
        ( Base::params.template getMeanY<0>() + dir.y()*2 );
    params.template setMeanY<1>
        ( Base::params.template getMeanY<0>() - dir.y()*2 );
    params.template setAmplitude<0>
        ( Base::params.template getAmplitude<0>() / 2 );
    params.template setAmplitude<1>
        ( Base::params.template getAmplitude<0>() / 2 );
    if ( FS ) {
        params.template setSigmaX<0>( this->start_sx );
        params.template setSigmaX<1>( this->start_sx );
        params.template setSigmaY<0>( this->start_sy );
        params.template setSigmaY<1>( this->start_sy );
        params.template setSigmaXY<0>( this->start_sxy );
        params.template setSigmaXY<1>( this->start_sxy );
    }

}

template class Width_Invariants<false,false>;
template class Width_Invariants<false,true>;
template class Width_Invariants<true,false>;
template class Width_Invariants<true,true>;

}
