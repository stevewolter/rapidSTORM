#include "debug.h"
#include "Fitter.h"
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include "Config.h"
#include "fitter/MarquardtInfo_impl.h"
#include <boost/units/pow.hpp>
#include "Exponential3D_Accessor.h"
#include <boost/units/cmath.hpp>
#include <fit++/FitFunction_impl.hh>
#include "fitter/residue_analysis/impl.h"

template <typename Ty>
inline Ty sqr(const Ty& a) { return a*a; }

namespace dStorm {
namespace fitter {
namespace residue_analysis {
template class CommonInfo<gauss_3d_fitter::ResidueAnalysisInfo>;
}
}

namespace gauss_3d_fitter {

using namespace fitpp::Exponential3D;
using dStorm::engine::Spot;
using dStorm::engine::BaseImage;
using boost::units::pow;

template <int Ks>
CommonInfo<Ks>::CommonInfo( 
   const Config& c, const engine::JobInfo& info
) 
: fitter::MarquardtInfo<FitGroup::VarC>(c,info),
  amplitude_threshold( *info.config.amplitude_threshold() ),
  params( new typename FitGroup::Accessor(NULL) ),
  constants( params->getConstants() )
{
    FitGroup::template Parameter<MeanX>::set_absolute_epsilon
        (this->fit_function, c.negligibleStepLength());
    FitGroup::template Parameter<MeanY>::set_absolute_epsilon
        (this->fit_function, c.negligibleStepLength());
    FitGroup::template Parameter<MeanZ>::set_absolute_epsilon
        (this->fit_function, c.negligibleStepLength());

    params->template set_all_ZAtBestSigmaX( - c.z_distance() / 2.0f );
    params->template set_all_ZAtBestSigmaY( c.z_distance() / 2.0f );
    params->template set_all_DeltaSigmaX( c.defocus_constant_x() );
    params->template set_all_DeltaSigmaY( c.defocus_constant_y() );
    params->template set_all_BestVarianceX( pow<2>(info.config.sigma_x()) );
    params->template set_all_BestVarianceY( pow<2>(info.config.sigma_y()) );
}

template <int Kernels>
CommonInfo<Kernels>::CommonInfo( const CommonInfo& o ) 
: fitter::MarquardtInfo<FitGroup::VarC>(o),
  maxs(o.maxs), start(o.start), amplitude_threshold(o.amplitude_threshold),
  params( new typename FitGroup::Accessor(*o.params) ),
  constants( params->getConstants() )
{
}

template <int Kernels>
void
CommonInfo<Kernels>::set_start(
    const Spot& spot, 
    const BaseImage& image,
    double shift_estimate_in_ADC,
    typename FitGroup::Variables* variables 
) 
{
    typedef boost::units::quantity<cs_units::camera::intensity,double>
        ADCs;

    ADCs shift_estimate = shift_estimate_in_ADC * cs_units::camera::ad_count;
    params->change_variable_set( variables );
    params->template set_all_MeanX( spot.x() * cs_units::camera::pixel );
    params->template set_all_MeanY( spot.y() * cs_units::camera::pixel );
    params->template set_all_MeanZ( 0 * boost::units::si::nanometre );

    int xc = round(spot.x()), yc = round(spot.y());
    ADCs center = image(xc,yc) * cs_units::camera::ad_count;
    
    params->setShift( shift_estimate );
    typename FitGroup::Accessor::QuantityBestVarianceX
        x_widening = pow<2>( params->template getDeltaSigmaX<0>() ) *
                pow<2>( params->template getZAtBestSigmaX<0>() );
    typename FitGroup::Accessor::QuantityBestVarianceX
        y_widening = pow<2>( params->template getDeltaSigmaY<0>() ) *
                pow<2>( params->template getZAtBestSigmaY<0>() );
    double prefactor = 2 * M_PI * 
        sqrt( ( 
            ( x_widening + params->template getBestVarianceX<0>() ) 
            * ( y_widening + params->template getBestVarianceY<0>() )
            ).value() );
    params->template set_all_Amplitude( 
        std::max<typename FitGroup::Accessor::QuantityAmplitude>
            (center - shift_estimate, 10 * cs_units::camera::ad_count)
            * prefactor );

    DEBUG( "Estimating center at " << center << ", shift at " << shift_estimate 
              << " for spot at " << xc << " " << yc << " with image sized " << image.width_in_pixels() << " by "
              << image.height_in_pixels() );
    DEBUG( "Set start " << *variables );

    maxs.x() = image.width_in_pixels()-1 - 1;
    maxs.y() = image.height_in_pixels()-1 - 1;
    start.x() = spot.x();
    start.y() = spot.y();
}

template <int Kernels>
bool 
CommonInfo<Kernels>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target
)
{
    params->change_variable_set( variables );
    DEBUG("Checking position " << variables->transpose());

    Localization::Position p;
    p.x() = params->template getMeanX<0>();
    p.y() = params->template getMeanY<0>();

    new(target) Localization( 
        p, Localization::Amplitude(params->template getAmplitude<0>()) );

    target->zposition() = 
        Localization::ZPosition(params->template getMeanZ<0>());

    DEBUG("Found position at " << p.x() << " " << p.y() << " " << target->zposition() << " " << target->strength());
    DEBUG("Amplitude threshold is " << amplitude_threshold);
    bool good = 
           target->strength() > amplitude_threshold 
        && target->x() >= 1*cs_units::camera::pixel
        && target->y() >= 1*cs_units::camera::pixel
        && target->x() < maxs.x() * cs_units::camera::pixel
        && target->y() < maxs.y() * cs_units::camera::pixel
        && sqr(target->x().value() - start.x()) + 
           sqr(target->y().value() - start.y()) < 4;

    DEBUG("Position good: " << good);
    target->unset_source_trace();
    return good;
}

template class CommonInfo<1>;
template class CommonInfo<2>;

ResidueAnalysisInfo::ResidueAnalysisInfo(
    const Config& config, 
    const engine::JobInfo& info
) 
: Base2(config,info),
  Base1(config,info)
{
    Base2::FitGroup::Parameter<Amplitude>::set_absolute_epsilon
        (this->Base2::fit_function, 1);
}

void ResidueAnalysisInfo::
start_from_splitted_single_fit(
    SingleFit& from,
    DoubleFit* v,
    const Eigen::Vector2i& dir
)
{
    Base1::params->change_variable_set( &from );
    Base2::params->change_variable_set( v );
    Base2::params->setShift( Base1::params->getShift() );

    using cs_units::camera::pixel;
    Base2::params->setMeanX<0>
        ( Base1::params->getMeanX<0>() + float(dir.x()) * pixel );
    Base2::params->setMeanX<1>
        ( Base1::params->getMeanX<0>() - float(dir.x()) * pixel );
    Base2::params->setMeanY<0>
        ( Base1::params->getMeanY<0>() + dir.y() * pixel );
    Base2::params->setMeanY<1>
        ( Base1::params->getMeanY<0>() - dir.y() * pixel );
    Base2::params->set_all_MeanZ( Base1::params->getMeanZ<0>() );
    Base2::params->setAmplitude<0>
        ( Base1::params->getAmplitude<0>() / 2.0 );
    Base2::params->setAmplitude<1>
        ( Base1::params->getAmplitude<0>() / 2.0 );
}

float
ResidueAnalysisInfo::sq_peak_distance( 
    DoubleFit *variables
) {
    Base2::params->change_variable_set( variables );
    
    float peak_dist =
        (Base2::params->getPosition<0>() - Base2::params->getPosition<1>())
         .start<2>().squaredNorm();
    return peak_dist;
}

void ResidueAnalysisInfo::get_center(
    const SingleFit& v, int& x, int& y) 
{
    Base1::params->change_variable_set( &const_cast<SingleFit&>(v) );
    x = round(Base1::params->getMeanX<0>() / cs_units::camera::pixel);
    y = round(Base1::params->getMeanY<0>() / cs_units::camera::pixel);
}


}
}
