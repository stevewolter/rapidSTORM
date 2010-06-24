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
void SizeSpecializing< Fitter< FreeForm_NoCorrelation, false > >
::create_specializations<0>()
{
}

template <>
template <>
void SizeSpecializing< Fitter< FixedForm, false > >
::create_specializations<1>();

template <>
template <>
void SizeSpecializing< Fitter< FixedForm, false > >
::create_specializations<0>()
{
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<5,6>();
#else
    this->make_specialization_array_entry<5,6>();
#endif
    create_specializations<1>();
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
