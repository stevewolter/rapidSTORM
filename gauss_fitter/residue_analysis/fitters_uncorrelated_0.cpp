#include "fitter/residue_analysis/fitter.h"
#include "main.h"
#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace fitter {

using namespace gauss_2d_fitter::residue_analysis;
using namespace fitpp::Exponential2D;

#define SPECIALIZE(x,i) \
template <> \
template <> \
void SizeSpecializing< residue_analysis::Fitter< Fitter<x, false > > > \
::create_specializations<i>()

SPECIALIZE(FreeForm_NoCorrelation,0) {
}

SPECIALIZE(FreeForm_NoCorrelation,1);

SPECIALIZE(FixedForm,0)
{
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<5,6>();
#else
    this->make_specialization_array_entry<11,10>();
#endif
    create_specializations<1>();
}
#undef SPECIALIZE

#define SPECIALIZE(x) \
template std::auto_ptr<Sized> \
    SizeSpecializing< residue_analysis::Fitter< Fitter<x, false > > > \
    ::make_unspecialized_fitter();

SPECIALIZE(FreeForm_NoCorrelation);
SPECIALIZE(FixedForm);

}
}
