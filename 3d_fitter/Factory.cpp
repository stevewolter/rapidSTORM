#include "debug.h"
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

template <int Widening>
Factory<Widening>::Factory() 
: simparm::Structure<Config<Widening> >(),
  SpotFitterFactory( static_cast<Config<Widening> &>(*this) )
{
}

template <int Widening>
Factory<Widening>::Factory(const Factory& c)
: simparm::Structure<Config<Widening> >(c), 
  SpotFitterFactory( static_cast<Config<Widening> &>(*this) )
{
}

template <int Widening>
Factory<Widening>::~Factory() {
}

template <int Widening>
std::auto_ptr<SpotFitter> 
Factory<Widening>::make (const engine::JobInfo &i)
{
    return fitter::residue_analysis::Fitter< gauss_3d_fitter::Fitter<Widening> >
    ::select_fitter(*this,i);
}

template <int Widening>
void Factory<Widening>::set_traits( output::Traits& rv, const engine::JobInfo& ) {
    DEBUG("3D fitter is setting traits");
    fitter::residue_analysis::Config::set_traits(rv);
    rv.covariance_matrix_is_set = false;
    rv.z_coordinate_is_set = true;
}

template class Factory< fitpp::Exponential3D::Holtzer >;
template class Factory< fitpp::Exponential3D::Zhuang >;

}
}

