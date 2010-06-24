#ifndef DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H
#define DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H

#include <fit++/Exponential2D.hh>
#include "no_analysis/main.h"

namespace dStorm {
namespace gauss_2d_fitter {
namespace residue_analysis {

template <int FitFlags>
struct CommonInfo
: public no_analysis::CommonInfo<FitFlags>
{
    typedef typename fitpp::Exponential2D::For<2, FitFlags> FitGroup;
    typename FitGroup::Constants constants;
    fitpp::FitFunction<FitGroup::VarC> fit_function;
    typename FitGroup::NamedParameters params;
    const double asymmetry_threshold, required_peak_distance_sq;

    CommonInfo( const Config&, const engine::JobInfo& );
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, typename FitGroup::Variables* variables );
    void start_from_splitted_single_fit
        ( typename FitGroup::Variables* v, const Eigen::Vector2i& dir);
    bool peak_distance_small(typename FitGroup::Variables *variables);
    bool check_result( typename FitGroup::Variables *variables, 
                       Localization *target);

    void set_two_kernel_improvement( Localization& l, float value );
};

template <int FitFlags, bool HonorCorrelation>
struct Fitter {
    typedef no_analysis::Fitter<FitFlags,HonorCorrelation> NoAnalysis;
    typedef CommonInfo<FitFlags> SizeInvariants;
    template <int X, int Y> struct Specialized;
};

template <class BaseFitter, int Width, int Height>
class SizedFitter;

template <int FitFlags, bool HonorCorrelation>
template <int X, int Y> 
struct Fitter<FitFlags,HonorCorrelation>::Specialized {
    typedef SizedFitter<Fitter,X,Y> Sized;
    typedef typename CommonInfo<FitFlags>::FitGroup
        ::template Deriver<engine::StormPixel,X,Y,HonorCorrelation> Deriver;
};

}
}
}

#endif
