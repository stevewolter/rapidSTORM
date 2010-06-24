#ifndef DSTORM_GAUSSFITTER_NOANALYSIS_H
#define DSTORM_GAUSSFITTER_NOANALYSIS_H

#include <fit++/Exponential2D.hh>

namespace dStorm {
namespace 2d_fitter {

template <int FitFlags>
struct NoAnalysis {
    class SizeInvariants;
    template <int X, int Y> struct SpecializationInfo;
};

template <int FitFlags>
struct NoAnalysis::SizeInvariants
{
    typedef typename fitpp::Exponential2D::For<1, FitFlags> FitGroup;

    typename FitGroup::Constants constants;
    FitFunction<FitGroup::VarC> fit_function;
    typename FitGroup::NamedParameters params;
    const double amplitude_threshold;
    const double start_sx, start_sy, start_sxy;

    Width_Invariants( const GaussFitterConfig&, const JobInfo& );
    fitter::StartInformation set_start( const Spot& spot, const BaseImage& image,
                    double shift_estimate,
                    typename FitGroup::Variables* variables );
    bool check_result( typename FitGroup::Variables *variables, 
                       Localization *target,
                       const StartInformation& start );
};

template <int FitFlags>
template <int X, int Y> 
struct NoAnalysis<FitFlags>::SpecializationInfo<X,Y> {
    typedef fitter::FixedSized<NoAnalysis,X,Y> 
        Sized;
    typedef fitpp::Exponential2D::For<1, FitFlags>::Deriver<engine::StormPixel,X,Y> 
        Deriver;
};

}
}

#endif
