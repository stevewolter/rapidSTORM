#define VERBOSE
#include "debug.h"
#include "Factory.h"
#include "Fitter.h"
#include <dStorm/output/Traits.h>
#include "fitter/SizeSpecializing_impl.h"
#include "fitter/residue_analysis/main.h"
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"
#include <dStorm/unit_interval.h>
#include <boost/lexical_cast.hpp>

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
    DEBUG("Destructing 3D fitter");
}

template <int Widening>
std::auto_ptr<SpotFitter> 
Factory<Widening>::make (const engine::JobInfo &i)
{
    DEBUG("Creating 3D fitter");
    if ( i.traits.size.z() == 1 * camera::pixel )
        return fitter::residue_analysis::Fitter< gauss_3d_fitter::Fitter<Widening,1> >
            ::select_fitter(*this,i);
    else if ( i.traits.size.z() == 2 * camera::pixel )
        return fitter::residue_analysis::Fitter< gauss_3d_fitter::Fitter<Widening,2> >
            ::select_fitter(*this,i);
    else {
        throw std::runtime_error("3D fitter not compiled for Z depth " 
            + boost::lexical_cast<std::string>( i.traits.size.z().value() ) );
    }
}

template <int Widening>
void Factory<Widening>::set_traits( output::Traits& rv, const engine::JobInfo& ) {
    DEBUG("3D fitter is setting traits");
    // TODO: CHeck traits */
    fitter::residue_analysis::Config::set_traits(rv);
    rv.covariance_matrix().is_given.diagonal().fill( this->output_sigmas() );
    rv.position().is_given.fill( true );
    rv.amplitude().is_given = true;
    rv.fit_residues().is_given = this->output_sigmas();

    Localization::Position::Traits::ValueType::Scalar range 
        = Localization::Position::Traits::ValueType::Scalar(Config<Widening>::z_range());
    Localization::Position::Traits::RangeType::Scalar p(-range, +range);
    rv.position().range().z() = p;
    DEBUG("3D fitter has set traits");
}

template <int Widening>
void Factory<Widening>::set_requirements( input::Traits<engine::Image>& )
{
    DEBUG("3D fitter has set requirements");
}

template class Factory< fitpp::Exponential3D::Holtzer >;
template class Factory< fitpp::Exponential3D::Zhuang >;

}
}

