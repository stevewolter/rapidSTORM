#ifndef DSTORM_FITTER_RESIDUEANALYSIS_H
#define DSTORM_FITTER_RESIDUEANALYSIS_H

#include <fit++/FitFunction.hh>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/Localization.h>
#include "Config.h"
#include "fitter/SizeSpecializing_decl.h"

namespace dStorm {
namespace fitter {
namespace residue_analysis {

template <class BaseInvariants>
struct CommonInfo
: public BaseInvariants
{
    typedef typename BaseInvariants::Config Config;
    const double asymmetry_threshold, required_peak_distance_sq;

    CommonInfo( const Config&, const engine::JobInfo& );
    bool peak_distance_small(typename BaseInvariants::DoubleFit *variables);

    void set_two_kernel_improvement( Localization& l, float value );
};

template <class BaseFitter>
struct Fitter {
    typedef CommonInfo<typename BaseFitter::SizeInvariants> SizeInvariants;
    template <int X, int Y> struct Specialized;

    static std::auto_ptr<engine::SpotFitter>
    select_fitter(const typename SizeInvariants::Config& config, 
                            const engine::JobInfo& info)
    {
        bool should_analyze = config.do_double_spot_analysis();
        if ( should_analyze )
            return fitter::create_SizeSpecializing<Fitter>(config,info);
        else 
            return fitter::create_SizeSpecializing
                < typename BaseFitter::OneKernel >(config,info);
    }
};

template <class BaseFitter, int Width, int Height>
class SizedFitter;

template <class BaseFitter>
template <int X, int Y> 
struct Fitter<BaseFitter>::Specialized {
    typedef SizedFitter<BaseFitter,X,Y> Sized;
    typedef typename BaseFitter::TwoKernel::
        template Specialized<X,Y>::Deriver Deriver;
};

}
}
}

#endif
