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
#include <dStorm/unit_matrix_operators.h>

template <typename Ty>
inline Ty sqr(const Ty& a) { return a*a; }

namespace dStorm {
namespace fitter {
namespace residue_analysis {
template class CommonInfo<gauss_3d_fitter::ResidueAnalysisInfo<fitpp::Exponential3D::Holtzer> >;
template class CommonInfo<gauss_3d_fitter::ResidueAnalysisInfo<fitpp::Exponential3D::Zhuang> >;
}
}

namespace gauss_3d_fitter {

using namespace fitpp::Exponential3D;
using dStorm::engine::Spot;
using dStorm::engine::BaseImage;
using boost::units::pow;

template <int Ks,int Widening>
NaiveFitter<Ks,Widening>::~NaiveFitter() {
    DEBUG("NaiveFitter destructor");
}

template class NaiveFitter<1, fitpp::Exponential3D::Zhuang>;
template class NaiveFitter<1, fitpp::Exponential3D::Holtzer>;
template class NaiveFitter<2, fitpp::Exponential3D::Zhuang>;
template class NaiveFitter<2, fitpp::Exponential3D::Holtzer>;

template <int Ks,int Widening>
CommonInfo<Ks,Widening>::CommonInfo( 
   const Config& c, const engine::JobInfo& info
) 
: fitter::MarquardtInfo<FitGroup::VarC>(c,info),
  amplitude_threshold( *info.config.amplitude_threshold() ),
  max_z_range( c.z_range() ),
  params( new typename FitGroup::Accessor(NULL) )
{
    DEBUG("Constructing fitter common information");
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
    params->template set_all_BestSigmaX( info.config.sigma_x() );
    params->template set_all_BestSigmaY( info.config.sigma_y() );

    for (int i = 0; i < 2; ++i)
        if ( info.traits.resolution[i].is_set() )
            scale_factor[i] = info.traits.resolution[i]->in_dpm();
        else {
            throw std::logic_error("Tried to use gauss fitter on image where pixel size is not given in nm.");
        }
    DEBUG("Constructed fitter common information");
}

template <int Kernels, int Widening>
CommonInfo<Kernels,Widening>::CommonInfo( const CommonInfo& o ) 
: fitter::MarquardtInfo<FitGroup::VarC>(o),
  maxs(o.maxs), start(o.start), amplitude_threshold(o.amplitude_threshold),
  max_z_range(o.max_z_range),
  params( new typename FitGroup::Accessor(*o.params) )
{
}

template <int Kernels, int Widening>
CommonInfo<Kernels,Widening>::~CommonInfo() {
    DEBUG("Destructing fitter common information");
}

template <int Kernels, int Widening>
const typename fitpp::Exponential3D::Model<Kernels,Widening>::Constants&
CommonInfo<Kernels,Widening>::constants() const
{ return params->getConstants(); };

template <int Kernels, int Widening>
void
CommonInfo<Kernels,Widening>::set_start(
    const Spot& spot, 
    const BaseImage& image,
    double shift_estimate_in_ADC,
    typename FitGroup::Variables* variables 
) 
{
    typedef boost::units::quantity<camera::intensity,double>
        ADCs;

    ADCs shift_estimate = shift_estimate_in_ADC * camera::ad_count;
    params->change_variable_set( variables );
    params->template set_all_MeanX( spot.x() * camera::pixel );
    params->template set_all_MeanY( spot.y() * camera::pixel );
    params->template set_all_MeanZ( 0 * boost::units::si::nanometre );

    int xc = round(spot.x()), yc = round(spot.y());
    ADCs center = image(xc,yc) * camera::ad_count;
    
    params->setShift( shift_estimate );

    double prefactor = 2 * M_PI * 
        (params->template getBestSigmaX<0>() * 
        params->template getBestSigmaY<0>()).value();
    params->template set_all_Amplitude( 
        std::max<typename FitGroup::Accessor::QuantityAmplitude>
            ((center - shift_estimate) / (1.0 * Kernels),
             10 * camera::ad_count) * prefactor );

    DEBUG( "Estimating center at " << center << ", shift at " << shift_estimate 
              << " for spot at " << xc << " " << yc << " with image sized " << image.width_in_pixels() << " by "
              << image.height_in_pixels() );
    DEBUG( "Set start " << *variables );

    maxs.x() = image.width() - 2 * camera::pixel;
    maxs.y() = image.height() - 2 * camera::pixel;
    start.x() = spot.x() * camera::pixel;
    start.y() = spot.y() * camera::pixel;
}

template <int Kernels, int Widening>
bool 
CommonInfo<Kernels,Widening>::check_result(
    typename FitGroup::Variables* variables,
    Localization* target
)
{
    params->change_variable_set( variables );
    DEBUG("Checking position " << variables->transpose());

    SubpixelPos mins, pixelpos;
    pixelpos[0] = params->template getMeanX<0>();
    pixelpos[1] = params->template getMeanY<0>();
    mins.fill( 1 * camera::pixel );

    Localization::Position::Type pos;
    pos[0] = pixelpos[0] / scale_factor[0];
    pos[1] = pixelpos[1] / scale_factor[1];
    pos[2] = Localization::Position::Type::Scalar(params->template getMeanZ<0>());
    new(target) Localization( 
        pos, Localization::Amplitude::Type(params->template getAmplitude<0>()) );

    DEBUG("Found position at " << p.x() << " " << p.y() << " " << target->zposition() << " " << target->strength());
    DEBUG("Amplitude threshold is " << amplitude_threshold);
    bool good = 
           target->amplitude() > amplitude_threshold 
        && (pixelpos.cwise() > mins).all() 
        && (pixelpos.cwise() < maxs).all();
    good = good
        && unitless_value(pixelpos - start).squaredNorm() < 4
        && target->position().z() < this->max_z_range
        && target->position().z() > - this->max_z_range;

    DEBUG("Position good: " << good);
    target->unset_source_trace();
    return good;
}

template class CommonInfo<1,fitpp::Exponential3D::Holtzer>;
template class CommonInfo<1,fitpp::Exponential3D::Zhuang>;
template class CommonInfo<2,fitpp::Exponential3D::Holtzer>;
template class CommonInfo<2,fitpp::Exponential3D::Zhuang>;

template <int Widening>
ResidueAnalysisInfo<Widening>::ResidueAnalysisInfo(
    const Config& config, 
    const engine::JobInfo& info
) 
: Base2(config,info),
  Base1(config,info)
{
    Base2::FitGroup::template Parameter<Amplitude>::set_absolute_epsilon
        (this->Base2::fit_function, 1);
}

template <int Widening>
void ResidueAnalysisInfo<Widening>::
start_from_splitted_single_fit(
    SingleFit& from,
    DoubleFit* v,
    const Eigen::Vector2i& dir
)
{
    Base1::params->change_variable_set( &from );
    Base2::params->change_variable_set( v );
    Base2::params->setShift( Base1::params->getShift() );

    using camera::pixel;
    Base2::params->template setMeanX<0>
        ( Base1::params->template getMeanX<0>() + float(dir.x()) * pixel );
    Base2::params->template setMeanX<1>
        ( Base1::params->template getMeanX<0>() - float(dir.x()) * pixel );
    Base2::params->template setMeanY<0>
        ( Base1::params->template getMeanY<0>() + dir.y() * pixel );
    Base2::params->template setMeanY<1>
        ( Base1::params->template getMeanY<0>() - dir.y() * pixel );
    Base2::params->set_all_MeanZ( Base1::params->template getMeanZ<0>() );
    Base2::params->set_all_Amplitude
        ( Base1::params->template getAmplitude<0>() / 2.0 );
}

template <int Widening>
float
ResidueAnalysisInfo<Widening>::sq_peak_distance( 
    DoubleFit *variables
) {
    Base2::params->change_variable_set( variables );
    
    float peak_dist =
        (Base2::params->template getPosition<0>() - Base2::params->template getPosition<1>())
         .template start<2>().squaredNorm();
    return peak_dist;
}

template <int Widening>
void ResidueAnalysisInfo<Widening>::get_center(
    const SingleFit& v, int& x, int& y) 
{
    Base1::params->change_variable_set( &const_cast<SingleFit&>(v) );
    x = round(Base1::params->template getMeanX<0>() / camera::pixel);
    y = round(Base1::params->template getMeanY<0>() / camera::pixel);
}

template class ResidueAnalysisInfo<fitpp::Exponential3D::Zhuang>;
template class ResidueAnalysisInfo<fitpp::Exponential3D::Holtzer>;

}
}
