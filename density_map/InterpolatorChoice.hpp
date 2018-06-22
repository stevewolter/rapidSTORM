#include "density_map/InterpolatorChoice.h"
#include "density_map/LinearInterpolation.h"
#include "density_map/NearestNeighbourInterpolation.h"
#include "density_map/GaussianSmoothing.h"

namespace dStorm {
namespace density_map {

template <int Dim>
InterpolatorChoice<Dim>::InterpolatorChoice() 
: choice("Interpolator", "Interpolation method")
{
    choice.addChoice( make_linear_interpolator_factory<Dim>() );
    choice.addChoice( make_nearest_neighbour_interpolator_factory<Dim>() );
    choice.addChoice( make_gaussian_smoothed_interpolator_factory<Dim>() );
    choice.set_user_level( simparm::Expert );
}

template <int Dim>
void InterpolatorChoice<Dim>::attach_ui( simparm::NodeHandle at ) 
    { choice.attach_ui( at ); }

template <int Dim>
std::auto_ptr< Interpolator<Dim> > InterpolatorChoice<Dim>::make() const {
    return choice().make_interpolator();
}

}
}

