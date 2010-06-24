#include "Factory.h"
#include "no_analysis/main.h"
#include "residue_analysis/main.h"
#include <dStorm/output/Traits.h>
#include "fitter/SizeSpecializing_impl.h"

namespace dStorm {
namespace gauss_2d_fitter {

using engine::SpotFitter;
using fitter::SizeSpecializing;

template <class Fitter>
std::auto_ptr<SpotFitter>
make_size_specializer(const Config& c, const engine::JobInfo& i)
{
    return std::auto_ptr<SpotFitter>(
        new SizeSpecializing<Fitter>(typename Fitter::SizeInvariants(c,i), i) );
}


template <int FitFlags, bool HonorCorrelation> 
std::auto_ptr<SpotFitter>
instantiate_Gauss_Fitter(
    const Config& c,
    const engine::JobInfo& i, 
    bool try_analysis = true) 
{
    bool should_analyze = (c.asymmetry_threshold() <= 0.99 );
    typedef residue_analysis::Fitter<FitFlags,HonorCorrelation> WithResidues;
    typedef no_analysis::Fitter<FitFlags,HonorCorrelation> WithoutResidues;

    if ( try_analysis && should_analyze )
        return std::auto_ptr<SpotFitter>( 
            make_size_specializer<WithResidues>(c,i) );
    else if ( !try_analysis && !should_analyze )
        return std::auto_ptr<SpotFitter>( 
            make_size_specializer<WithoutResidues>(c,i) );
    else if ( try_analysis )
        return instantiate_Gauss_Fitter<FitFlags,HonorCorrelation>(c, i, false);
    else
        throw std::runtime_error("Recursion base missed in select_gauss_fitter.");
}

Factory::Factory() 
: simparm::Structure<Config>(),
  SpotFitterFactory( static_cast<Config&>(*this) )
{
}

Factory::Factory(const Factory& c)
: simparm::Structure<Config>(c), 
  SpotFitterFactory( static_cast<Config&>(*this) )
{
}

Factory::~Factory() {
}

std::auto_ptr<SpotFitter> 
Factory::make (const engine::JobInfo &i)
{
    bool free_sigmas = freeSigmaFitting();
    bool fit_correlation = free_sigmas && !fixCorrelationTerm();
    bool ignore_correlation = !fit_correlation && sigma_xy_negligible_limit() >= abs( i.config.sigma_xy() );

    if ( free_sigmas )
        if ( !fixCorrelationTerm() )
            return instantiate_Gauss_Fitter<fitpp::Exponential2D::FreeForm,true>( *this, i );
        else if ( ignore_correlation )
            return instantiate_Gauss_Fitter<fitpp::Exponential2D::FreeForm_NoCorrelation,false>( *this, i );
        else
            return instantiate_Gauss_Fitter<fitpp::Exponential2D::FreeForm_NoCorrelation,true>( *this, i );
    else if ( ignore_correlation )
        return instantiate_Gauss_Fitter<fitpp::Exponential2D::FixedForm,false>( *this, i );
    else
        return instantiate_Gauss_Fitter<fitpp::Exponential2D::FixedForm,true>( *this, i );
}

void Factory::set_traits( output::Traits& rv ) {
    rv.two_kernel_improvement_is_set = (asymmetry_threshold() < 1.0);
    rv.covariance_matrix_is_set = freeSigmaFitting();
}

}
}

