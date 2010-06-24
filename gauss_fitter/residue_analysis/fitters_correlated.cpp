#include "main.h"
#include "fitter.h"
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
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
void SizeSpecializing< Fitter< FreeForm, true > >::
create_specializations<0>()
{
}

template <>
template <>
void SizeSpecializing< Fitter< FreeForm_NoCorrelation, true > >::
create_specializations<0>()
{
}

template <>
template <>
void SizeSpecializing< Fitter< FixedForm, true > >::
create_specializations<0>()
{
}

template std::auto_ptr<Sized>
    SizeSpecializing< Fitter< FreeForm, true> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< Fitter< FreeForm_NoCorrelation, true> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< Fitter< FixedForm, true> >
    ::make_unspecialized_fitter();
}
}
