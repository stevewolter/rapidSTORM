#include "GaussFitterFactory.h"
#include "GaussFitter_impl.h"
#include <dStorm/output/Traits.h>

namespace dStorm {
namespace engine {

template <int Mask> 
SpotFitter* instantiate_Gauss_Fitter(int mask, const GaussFitterConfig& c, const JobInfo& i) {
    if ( mask == Mask )
        return new GaussFitter< Mask & 0x1, Mask & 0x2, Mask & 0x4 >(c, i);
    else if ( Mask > 0 )
        return instantiate_Gauss_Fitter<(Mask==0)?0:Mask-1>(mask, c, i);
    else
        throw std::runtime_error("Recursion base missed "
                                 "in select_gauss_fitter.");
}

GaussFitterFactory::GaussFitterFactory() 
: simparm::Structure<GaussFitterConfig>(),
  SpotFitterFactory( static_cast<GaussFitterConfig&>(*this) )
{
}

GaussFitterFactory::GaussFitterFactory(const GaussFitterFactory& c)
: simparm::Structure<GaussFitterConfig>(c), 
  SpotFitterFactory( static_cast<GaussFitterConfig&>(*this) )
{
}

GaussFitterFactory::~GaussFitterFactory() {
}

std::auto_ptr<SpotFitter> 
GaussFitterFactory::make (const JobInfo &i)
{
    bool residue_analysis = ( asymmetry_threshold() <= 0.99 );
    bool free_sigmas = freeSigmaFitting();
    bool correlation = (free_sigmas) 
        ? (!fixCorrelationTerm())
        : sigma_xy_negligible_limit() <= abs( i.config.sigma_xy() );

    int mask = (residue_analysis ? 0x2 : 0) + 
               (free_sigmas ? 0x1 : 0) +
               (correlation ? 0x4 : 0);
    return std::auto_ptr<SpotFitter>( instantiate_Gauss_Fitter<7>(mask, *this, i) );
}

void GaussFitterFactory::set_traits( output::Traits& rv ) {
    rv.two_kernel_improvement_is_set = (asymmetry_threshold() < 1.0);
    rv.covariance_matrix_is_set = freeSigmaFitting();
}

}
}
