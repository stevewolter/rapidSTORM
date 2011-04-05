#include "no_analysis/main.h"
#include "residue_analysis/main.h"
#include "fitter/residue_analysis/main.h"
#include "fitter/SizeSpecializing_impl.h"
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"
#include "Factory.h"
#include <dStorm/output/Traits.h>

namespace dStorm {
namespace gauss_2d_fitter {

namespace spot_fitter = engine::spot_fitter;
using fitter::SizeSpecializing;

Factory::Factory() 
: simparm::Structure<Config>(),
  spot_fitter::Factory( static_cast<Config&>(*this) )
{
}

Factory::Factory(const Factory& c)
: simparm::Structure<Config>(c), 
  spot_fitter::Factory( static_cast<Config&>(*this) )
{
}

Factory::~Factory() {
}

template <int FitFlags,bool HonorCorrelation>
std::auto_ptr<spot_fitter::Implementation>
instantiate(const Config& c, const engine::JobInfo& i)
{
    return fitter::residue_analysis::Fitter<
            residue_analysis::Fitter<FitFlags,HonorCorrelation> >
        ::select_fitter(c,i);
};

using namespace fitpp::Exponential2D;

std::auto_ptr<spot_fitter::Implementation> 
Factory::make (const engine::JobInfo &i)
{
    bool correlation_negligible = true;

    if ( freeSigmaFitting() )
        if ( fixCorrelationTerm() )
            if ( correlation_negligible )
                return instantiate<FreeForm_NoCorrelation,false>(*this,i);
            else
                return instantiate<FreeForm_NoCorrelation,true>(*this, i);
        else 
            return instantiate<FreeForm,true>( *this, i );
    else if ( correlation_negligible )
        return instantiate<FixedForm,false>( *this, i );
    else
        return instantiate<FixedForm,true>( *this, i );
}

void Factory::set_requirements( input::Traits<engine::Image>& t ) {
    t.photon_response.require( deferred::JobTraits );
    t.background_stddev.require( deferred::JobTraits );
}

void Factory::set_traits( output::Traits& rv, const engine::JobInfo& info ) {
    fitter::residue_analysis::Config::set_traits(rv);
    rv.covariance_matrix().is_given.fill( freeSigmaFitting() );
    rv.position().is_given.start<2>().fill( true );
    rv.amplitude().is_given = true;
    rv.fit_residues().is_given = true;

    if ( info.traits.photon_response.is_promised(deferred::JobTraits)
         && info.traits.background_stddev.is_promised(deferred::JobTraits) ) 
    {
        rv.position().uncertainty_is_given.start<2>().fill( true );
    }
}

}
}

