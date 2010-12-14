#ifndef DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H
#define DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H

#include <cs_units/camera/intensity.hpp>
#include "fitter/MarquardtInfo.h"
#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/Localization_decl.h>
#include "fitter/FixedSized_decl.h"
#include <memory>
#include "Config_decl.h"
#include "Exponential3D.hh"
#include <boost/units/quantity.hpp>
#include <dStorm/units/nanolength.h>

namespace dStorm {
namespace gauss_3d_fitter {

template <int Kernels, int Widening>
class CommonInfo
: public fitter::MarquardtInfo
    <fitpp::Exponential3D::Model<Kernels,Widening>::VarC>
{
  protected:
    typedef typename fitpp::Exponential3D::Model<Kernels,Widening> FitGroup;
    typedef typename FitGroup::Variables Variables;
    Eigen::Vector2i maxs;
    Eigen::Vector2d start;
    const boost::units::quantity<cs_units::camera::intensity> amplitude_threshold;
    const boost::units::quantity<boost::units::si::nanolength> max_z_range;

  public:
    std::auto_ptr<typename FitGroup::Accessor> params;
    const typename FitGroup::Constants& constants() const;

 public:
    typedef gauss_3d_fitter::Config<Widening> Config;
    CommonInfo( const Config&, const engine::JobInfo& );
    CommonInfo( const CommonInfo& );
    ~CommonInfo();
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, Variables* variables );
    bool check_result( Variables *variables, Localization *target);
};

template <int Widening>
class ResidueAnalysisInfo
: public CommonInfo<2,Widening>, public CommonInfo<1,Widening>
{
    typedef CommonInfo<1,Widening> Base1;
    typedef CommonInfo<2,Widening> Base2;
  public:
    typedef typename Base1::FitGroup::Variables SingleFit;
    typedef typename Base2::FitGroup::Variables DoubleFit;

    typedef gauss_3d_fitter::Config<Widening> Config;

    ResidueAnalysisInfo( const Config&, const engine::JobInfo& );

    void start_from_splitted_single_fit
        ( SingleFit& from, DoubleFit* v, 
          const Eigen::Vector2i& dir);
    float sq_peak_distance(DoubleFit *variables);
    void get_center( const SingleFit& v, int& x, int& y);
};


template <int Kernels, int Widening>
struct NaiveFitter {
    typedef CommonInfo<Kernels,Widening> SizeInvariants;
    typedef fitpp::Exponential3D::Model<Kernels,Widening> Model;

    template <int X, int Y>
    struct Specialized {
        typedef fitter::FixedSized<NaiveFitter,X,Y> Sized;
        typedef typename Model::template Fitter<engine::StormPixel,X,Y>
            ::Type Deriver;
    };

    ~NaiveFitter();
};

template <int Widening>
struct Fitter {
    typedef ResidueAnalysisInfo<Widening> SizeInvariants;
    typedef NaiveFitter<1,Widening> OneKernel;
    typedef NaiveFitter<2,Widening> TwoKernel;
};

}
}

#endif
