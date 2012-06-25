#ifndef DSTORM_DENSITY_MAP_LINEAR_INTERPOLATION_HPP
#define DSTORM_DENSITY_MAP_LINEAR_INTERPOLATION_HPP

#include "LinearInterpolation.h"
#include "Interpolator.h"
#include "InterpolatorFactory.h"
#include <bitset>
#include <simparm/Object.h>

namespace dStorm {
namespace density_map {

template <int Dim>
class LinearInterpolator : public Interpolator<Dim>
{
    typedef typename Interpolator<Dim>::ResultPoint ResultPoint;
    typedef typename Interpolator<Dim>::Position Position;
    LinearInterpolator* clone_() const { return new LinearInterpolator(*this); }
    void interpolate_( 
        const dStorm::Localization&, const Eigen::Array<float,Dim,1>& values,
        std::vector<ResultPoint>& target ) const 
    {
        target.clear();
        Eigen::Array<float,Dim,1> lower, terms;
        lower = floor( values );

        const Position base_pos 
            = from_value<camera::length>(lower.template cast<int>());

        /* This loops iterates over the linear interpolation terms,
         * i.e. the corners of the hypercube. */
        for (unsigned int corner_ = 0; corner_ < (1u << Dim); ++corner_) {
            Position p = base_pos;
            std::bitset<Dim> high_corner( corner_ );
            for (int j = 0; j < Dim; ++j)
                if ( high_corner[j] ) p[j] += 1 * camera::pixel;
            for (int j = 0; j < Dim; ++j)
                    terms[j] = (high_corner[j]) ? (values[j] - lower[j]) : 1.0f - (values[j] - lower[j]);

            target.push_back( ResultPoint( p, std::abs( terms.prod() ) ) );
        }

    }
};

template <int Dim>
std::auto_ptr< Interpolator<Dim> > make_linear_interpolator() {
    return std::auto_ptr< Interpolator<Dim> >( new LinearInterpolator<Dim>() );
}

template <int Dim>
class LinearInterpolatorFactory : public InterpolatorFactory<Dim> {
    simparm::Object name_object;

    LinearInterpolatorFactory* clone() const { return new LinearInterpolatorFactory(*this); }
    std::string getName() const { return name_object.getName(); }
    void attach_ui( simparm::NodeHandle to ) { name_object.attach_ui( to ); }
    Interpolator<Dim>* make_interpolator_() const { return new LinearInterpolator<Dim>(); }

public:
    LinearInterpolatorFactory() : name_object("LinearInterpolation", "Linear interpolation") {}
};

template <int Dim>
std::auto_ptr< InterpolatorFactory<Dim> > make_linear_interpolator_factory() {
    return std::auto_ptr< InterpolatorFactory<Dim> >( new LinearInterpolatorFactory<Dim>() );
}

}
}

#endif
