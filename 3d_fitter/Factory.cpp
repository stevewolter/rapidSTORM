#include "Factory.h"
#include "Fitter.h"
#include <dStorm/output/Traits.h>
#include "fitter/SizeSpecializing_impl.h"
#include "fitter/residue_analysis/main.h"
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"

namespace dStorm {
namespace gauss_3d_fitter {

using engine::SpotFitter;
using fitter::SizeSpecializing;

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
    return fitter::residue_analysis::Fitter< gauss_3d_fitter::Fitter >
        ::select_fitter(*this,i);
}

void Factory::set_traits( output::Traits& rv ) {
    rv.two_kernel_improvement_is_set = false;
    rv.covariance_matrix_is_set = false;
}

}
}

