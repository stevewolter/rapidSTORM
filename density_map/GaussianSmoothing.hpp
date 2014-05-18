#ifndef DSTORM_DENSITY_MAP_GAUSSIAN_SMOOTHINGN_HPP
#define DSTORM_DENSITY_MAP_GAUSSIAN_SMOOTHINGN_HPP

#include "density_map/GaussianSmoothing.h"
#include "density_map/Interpolator.h"
#include "density_map/InterpolatorFactory.h"
#include <bitset>
#include "simparm/Object.h"
#include "simparm/Entry.h"
#include <boost/range/numeric.hpp>
#include <boost/bind/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/math/constants/constants.hpp>

namespace dStorm {
namespace density_map {

template <int Dim>
class GaussianSmoothingInterpolator : public Interpolator<Dim>
{
public:
    GaussianSmoothingInterpolator( float sigmas, float range ) : sigmas(sigmas), range(range) {}
private:
    typedef typename Interpolator<Dim>::ResultPoint ResultPoint;
    typedef typename Interpolator<Dim>::Position Position;
    GaussianSmoothingInterpolator* clone_() const { return new GaussianSmoothingInterpolator(*this); }

    const float sigmas, range;

    void apply_factors( const Position& base_pos, int d, const Eigen::Array<float,Dim,1>& v, const Eigen::Array<float,Dim,1> u, float factor, std::vector<ResultPoint>& target ) const {
        if ( d == Dim ) {
            target.push_back( ResultPoint( base_pos, factor ) );
        } else {
            float sigma = u[d] * sigmas, radius = range * sigma;
            if ( radius >= 0.5 ) {
                int lower = floor( v[d] - radius ), upper = ceil( v[d] + radius );
                Position p = base_pos;
                for (int i = lower; i <= upper; ++i) {
                    p[d] = i * camera::pixel;
                    float subfactor = exp( -0.5 * pow<2>( (i - v[d]) / sigma ) ) / sqrt( 2 * boost::math::constants::pi<double>() * sigma );
                    apply_factors( p, d + 1, v, u, factor * subfactor, target );
                }
            } else {
                Position p = base_pos;
                p[d] = int(round( v[d] )) * camera::pixel;
                apply_factors( p, d + 1, v, u, factor, target );
            }
        }
    }

    void interpolate_( 
        const Eigen::Array<float,Dim,1>& values, const Eigen::Array<float,Dim,1>& uncertainty,
        std::vector<ResultPoint>& target ) const 
    {
        target.clear();
        apply_factors( Position(), 0, values, uncertainty, 1.0f, target );
        float sum = 0.0f;
        BOOST_FOREACH( ResultPoint& p, target )
            sum += p.relative_value;
        BOOST_FOREACH( ResultPoint& p, target )
            p.relative_value /= sum;
    }
};

template <int Dim>
class GaussianSmoothingInterpolatorFactory : public InterpolatorFactory<Dim> {
    simparm::Object name_object;
    simparm::Entry<double> sigmas, range;

    GaussianSmoothingInterpolatorFactory* clone() const { return new GaussianSmoothingInterpolatorFactory(*this); }
    std::string getName() const { return name_object.getName(); }
    void attach_ui( simparm::NodeHandle to ) { 
        simparm::NodeHandle r = name_object.attach_ui( to ); 
        sigmas.attach_ui( r );
        range.attach_ui( r );
    }
    Interpolator<Dim>* make_interpolator_() const { return new GaussianSmoothingInterpolator<Dim>( sigmas(), range() ); }

public:
    GaussianSmoothingInterpolatorFactory() 
        : name_object("GaussianSmoothing", "Smoothed by localization precision"),
          sigmas("SmoothingFactor", "Smoothing factor", 1.0),
          range("Range", "Width of kernel", 3.0) {}
};

template <int Dim>
std::auto_ptr< InterpolatorFactory<Dim> > make_gaussian_smoothed_interpolator_factory() {
    return std::auto_ptr< InterpolatorFactory<Dim> >( new GaussianSmoothingInterpolatorFactory<Dim>() );
}

}
}

#endif
