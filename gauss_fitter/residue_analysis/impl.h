#ifndef RESIDUEANALYSIS_IMPL_H
#define RESIDUEANALYSIS_IMPL_H
#include "main.h"
#include "no_analysis/main.h"
#include "fitter/residue_analysis/impl.h"
#include <fit++/FitFunction_impl.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include <dStorm/engine/Input.h>
#include "Config.h"
#include <Eigen/Array>

namespace dStorm {
namespace gauss_2d_fitter {
namespace residue_analysis {

using namespace fitpp::Exponential2D;

template <int FF>
CommonInfo<FF>::CommonInfo(
    const Config& config, 
    const engine::JobInfo& info
) 
: Base2(config,info),
  Base1(config,info)
{
    Base2::FitGroup::template Parameter<Amplitude>::set_absolute_epsilon
        (this->Base2::fit_function, 1);
}


template <int FF>
void CommonInfo<FF>::
start_from_splitted_single_fit(
    SingleFit& from,
    DoubleFit* v,
    const Eigen::Vector2i& dir
)
{
    Base1::params.change_variable_set( &from );
    Base2::set_start(v);
    Base2::params.setShift( Base1::params.getShift() );

    Base2::params.template setMeanX<0>
        ( Base1::params.template getMeanX<0>() + dir.x() );
    Base2::params.template setMeanX<1>
        ( Base1::params.template getMeanX<0>() - dir.x() );
    Base2::params.template setMeanY<0>
        ( Base1::params.template getMeanY<0>() + dir.y() );
    Base2::params.template setMeanY<1>
        ( Base1::params.template getMeanY<0>() - dir.y() );
    Base2::params.template setAmplitude<0>
        ( Base1::params.template getAmplitude<0>() / 2 );
    Base2::params.template setAmplitude<1>
        ( Base1::params.template getAmplitude<0>() / 2 );
}

template <int FF>
void CommonInfo<FF>::
get_center(const SingleFit& position, int& x, int& y)
{
    Base1::params.change_variable_set( const_cast<SingleFit*>(&position) );
    x = round(Base1::params.template getMeanX<0>());
    y = round(Base1::params.template getMeanY<0>());
}

template <int FF>
float
CommonInfo<FF>::sq_peak_distance( 
    DoubleFit *variables
) {
    Base2::params.change_variable_set( variables );
    
    float peak_dist =
        (Base2::params.template getPosition<0>()
         - Base2::params.template getPosition<1>())
         .template start<2>().squaredNorm();
    return peak_dist;
}

}
}
}

#endif
