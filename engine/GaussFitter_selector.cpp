#include "GaussFitterFactory.h"
#include "engine/GaussFitter_impl.h"

namespace dStorm {
namespace engine {

template <int Mask> 
SpotFitter* instantiate_Gauss_Fitter(int mask, const GaussFitterConfig& c, const JobInfo& i) {
    if ( mask == Mask )
        return new GaussFitter< Mask & 0x1, Mask & 0x2, Mask & 0x5 >(c, i);
    else if ( Mask > 0 )
        return instantiate_Gauss_Fitter<(Mask==0)?0:Mask-1>(mask, c, i);
    else
        throw std::runtime_error("Recursion base missed "
                                 "in select_gauss_fitter.");
}

GaussFitterConfig::GaussFitterConfig() 
: simparm::Set("GaussFitter", "Config for Gaussian fitter")
{
}

GaussFitterFactory::GaussFitterFactory() 
: simparm::Structure<GaussFitterConfig>(),
  SpotFitterFactory( static_cast<GaussFitterConfig&>(*this) )
{
}

std::auto_ptr<SpotFitter> 
GaussFitterFactory::make (const JobInfo &i)
{
    bool residue_analysis = ( i.config.asymmetry_threshold() <= 0.99 );
    bool free_sigmas = i.config.freeSigmaFitting();
    bool correlation = free_sigmas || 
        i.config.sigma_xy_negligible_limit() <= abs( i.config.sigma_xy() );

    int mask = (residue_analysis ? 0x2 : 0) + 
               (free_sigmas ? 0x1 : 0) +
               (correlation ? 0x4 : 0);
    return std::auto_ptr<SpotFitter>( instantiate_Gauss_Fitter<7>(mask, *this, i) );
}

}
}
