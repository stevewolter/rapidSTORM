#ifndef DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H
#define DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H

#include "fitter/MarquardtInfo.h"
#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/Localization_decl.h>
#include "fitter/FixedSized_decl.h"
#include <memory>
#include "Config_decl.h"
#include "Exponential3D.hh"

namespace dStorm {
namespace gauss_3d_fitter {

template <int Kernels>
class CommonInfo
: public fitter::MarquardtInfo
    <fitpp::Exponential3D::Model<Kernels>::VarC>
{
  protected:
    typedef typename fitpp::Exponential3D::Model<Kernels> FitGroup;
    typedef typename FitGroup::Variables Variables;
    Eigen::Vector2i maxs;
    Eigen::Vector2d start;
    const double amplitude_threshold;

  public:
    std::auto_ptr<typename FitGroup::Accessor> params;

 public:
    typedef gauss_3d_fitter::Config Config;
    CommonInfo( const Config&, const engine::JobInfo& );
    CommonInfo( const CommonInfo& );
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, Variables* variables );
    bool check_result( Variables *variables, Localization *target);
};

class ResidueAnalysisInfo
: public CommonInfo<2>, public CommonInfo<1>
{
    typedef CommonInfo<1> Base1;
    typedef CommonInfo<2> Base2;
  public:
    typedef Base1::FitGroup::Variables SingleFit;
    typedef Base2::FitGroup::Variables DoubleFit;

    typedef gauss_3d_fitter::Config Config;

    ResidueAnalysisInfo( const Config&, const engine::JobInfo& );

    void start_from_splitted_single_fit
        ( SingleFit& from, DoubleFit* v, 
          const Eigen::Vector2i& dir);
    float sq_peak_distance(DoubleFit *variables);
    void get_center( const SingleFit& v, int& x, int& y);
};


template <int Kernels>
struct NaiveFitter {
    typedef CommonInfo<Kernels> SizeInvariants;
    typedef fitpp::Exponential3D::Model<Kernels> Model;

    template <int X, int Y>
    struct Specialized {
        typedef fitter::FixedSized<NaiveFitter,X,Y> Sized;
        typedef typename Model::template Fitter<engine::StormPixel,X,Y>
            ::Type Deriver;
    };
};

struct Fitter {
    typedef ResidueAnalysisInfo SizeInvariants;
    typedef NaiveFitter<1> OneKernel;
    typedef NaiveFitter<2> TwoKernel;
};

}
}

#endif
