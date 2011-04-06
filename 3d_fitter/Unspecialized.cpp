#include "Fitter.h"
#include "fitter/residue_analysis/fitter.h"
#include "fit++/FitFunction_impl.hh"
#include "Exponential3D_Derivatives.h"
#include "fitter/SizeSpecializing_filler.h"

namespace dStorm {
namespace fitter {

using namespace fitpp::Exponential3D;
using namespace fitpp::Exponential3D;

#define INSTANTIATE_NAIVE_AND_RESANALYZE(f,d) \
template std::auto_ptr<Sized> \
    SizeSpecializing< residue_analysis::Fitter< gauss_3d_fitter::Fitter<f,d> > > \
    ::make_unspecialized_fitter(); \
template std::auto_ptr<Sized> \
    SizeSpecializing< gauss_3d_fitter::NaiveFitter<1,f,d> > \
    ::make_unspecialized_fitter();

INSTANTIATE_NAIVE_AND_RESANALYZE(Holtzer,1)
INSTANTIATE_NAIVE_AND_RESANALYZE(Holtzer,2)
INSTANTIATE_NAIVE_AND_RESANALYZE(Zhuang,1)
INSTANTIATE_NAIVE_AND_RESANALYZE(Zhuang,2)

}
}
