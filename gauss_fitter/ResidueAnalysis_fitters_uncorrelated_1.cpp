#include "ResidueAnalysis_fitter.h"
#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace fitter {

using namespace gauss_2d_fitter::residue_analysis;
using namespace fitpp::Exponential2D;

template <>
template <>
void SizeSpecializing<
    Fitter< FixedForm, false >
>::
create_specializations<1>()
{
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,4>();
#endif
}

}
}
