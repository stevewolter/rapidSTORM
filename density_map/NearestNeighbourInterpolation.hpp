#ifndef DSTORM_DENSITY_MAP_NEAREST_NEIGHBOUR_INTERPOLATION_HPP
#define DSTORM_DENSITY_MAP_NEAREST_NEIGHBOUR_INTERPOLATION_HPP

#include "NearestNeighbourInterpolation.h"
#include "Interpolator.h"
#include "InterpolatorFactory.h"
#include <bitset>
#include <simparm/Object.h>

namespace dStorm {
namespace density_map {

template <int Dim>
class NearestNeighbourInterpolator : public Interpolator<Dim>
{
    typedef typename Interpolator<Dim>::ResultPoint ResultPoint;
    typedef typename Interpolator<Dim>::Position Position;
    NearestNeighbourInterpolator* clone_() const { return new NearestNeighbourInterpolator(*this); }
    void interpolate_( 
        const dStorm::Localization&, const Eigen::Array<float,Dim,1>& values,
        std::vector<ResultPoint>& target ) const 
    {
        target.clear();
        const Position p
            = from_value<camera::length>( round( values ).template cast<int>());
        target.push_back( ResultPoint( p, 1.0f ) );
    }
};

template <int Dim>
std::auto_ptr< Interpolator<Dim> > make_nearest_neighbour_interpolator() {
    return std::auto_ptr< Interpolator<Dim> >( new NearestNeighbourInterpolator<Dim>() );
}

template <int Dim>
class NearestNeighbourInterpolatorFactory : public InterpolatorFactory<Dim> {
    simparm::Object name_object;

    NearestNeighbourInterpolatorFactory* clone() const { return new NearestNeighbourInterpolatorFactory(*this); }
    std::string getName() const { return name_object.getName(); }
    void attach_ui( simparm::NodeHandle to ) { name_object.attach_ui( to ); }
    Interpolator<Dim>* make_interpolator_() const { return new NearestNeighbourInterpolator<Dim>(); }

public:
    NearestNeighbourInterpolatorFactory() : name_object("NearestNeighbour", "Nearest neighbour") {}
};

template <int Dim>
std::auto_ptr< InterpolatorFactory<Dim> > make_nearest_neighbour_interpolator_factory() {
    return std::auto_ptr< InterpolatorFactory<Dim> >( new NearestNeighbourInterpolatorFactory<Dim>() );
}

}
}

#endif
