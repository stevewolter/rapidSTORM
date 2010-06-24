#include "NoAnalysis.h"
#include "fitter/FixedSized.h"
#include <fit++/Exponential2D_Uncorrelated_Derivatives.hh>
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace fitter {

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< 
        fitpp::Exponential2D::FreeForm_NoCorrelation, false >
>::
create_specializations<0>()
{
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,6>();
#endif
}

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< 
        fitpp::Exponential2D::FixedForm, false >
>::
create_specializations<0>()
{
#ifdef USE_SPECIALIZED_FITTERS
    this->fill_specialization_array<3,6>();
#endif
}

template std::auto_ptr<Sized>
    SizeSpecializing< Fitter< FreeForm_NoCorrelation, false> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< Fitter< FixedForm, false> >
    ::make_unspecialized_fitter();

}
}
