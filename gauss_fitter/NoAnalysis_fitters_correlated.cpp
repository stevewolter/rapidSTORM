#include "NoAnalysis.h"
#include "fitter/FixedSized.h"
#include <fit++/Exponential2D_Correlated_Derivatives.hh>
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {
namespace fitter {

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< fitpp::Exponential2D::FreeForm, true >
>::
create_specializations<0>()
{
}

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< fitpp::Exponential2D::FreeForm_NoCorrelation, true >
>::
create_specializations<0>()
{
}

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< fitpp::Exponential2D::FixedForm, true >
>::
create_specializations<0>()
{
}

template std::auto_ptr<Sized>
    SizeSpecializing< 
        gauss_2d_fitter::no_analysis::Fitter<
            fitpp::Exponential2D::FreeForm, true> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< 
        gauss_2d_fitter::no_analysis::Fitter<
            fitpp::Exponential2D::FreeForm_NoCorrelation, true> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< 
        gauss_2d_fitter::no_analysis::Fitter<
            fitpp::Exponential2D::FixedForm, true> >
    ::make_unspecialized_fitter();
}

}

