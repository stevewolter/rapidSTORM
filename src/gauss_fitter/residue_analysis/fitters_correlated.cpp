#include "main.h"
#include "fitter/residue_analysis/fitter.h"
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
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
void SizeSpecializing< residue_analysis::Fitter< Fitter<x, true > > > \
::create_specializations<i>()

SPECIALIZE(FreeForm,0) {}
SPECIALIZE(FreeForm_NoCorrelation,0) {}
SPECIALIZE(FixedForm,0) {}
#undef SPECIALIZE

#define SPECIALIZE(x) \
template std::auto_ptr<Sized> \
    SizeSpecializing< residue_analysis::Fitter< Fitter< x, true> > > \
    ::make_unspecialized_fitter();

SPECIALIZE(FreeForm)
SPECIALIZE(FreeForm_NoCorrelation)
SPECIALIZE(FixedForm)
#undef SPECIALIZE
}
}
