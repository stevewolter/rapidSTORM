#include "main.h"
#include "fit++/FitFunction_impl.hh"
#include "fitter/FixedSized_impl.h"
#include "fit++/Exponential2D_Uncorrelated_Derivatives.hh"
#include "fitter/SizeSpecializing_filler.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace Eigen {
    template <>
    class NumTraits<unsigned short>
        : public NumTraits<int> {};
}

namespace dStorm {
namespace fitter {

using namespace gauss_2d_fitter::no_analysis;
using namespace fitpp::Exponential2D;

template <>
template <>
void SizeSpecializing<
    gauss_2d_fitter::no_analysis::Fitter< FreeForm_NoCorrelation, false >
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
    SizeSpecializing< gauss_2d_fitter::no_analysis::Fitter< FreeForm_NoCorrelation, false> >
    ::make_unspecialized_fitter();
template std::auto_ptr<Sized>
    SizeSpecializing< gauss_2d_fitter::no_analysis::Fitter< FixedForm, false> >
    ::make_unspecialized_fitter();

}
}
