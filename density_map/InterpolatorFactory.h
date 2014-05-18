#ifndef DSTORM_DENSITY_MAP_INTERPOLATOR_FACTORY_H
#define DSTORM_DENSITY_MAP_INTERPOLATOR_FACTORY_H

#include "simparm/Choice.h"
#include "make_clone_allocator.hpp"

namespace dStorm {
namespace density_map {

template <int Dim> class Interpolator;

template <int Dim>
class InterpolatorFactory : public simparm::Choice {
    virtual Interpolator<Dim>* make_interpolator_() const = 0;
public:
    InterpolatorFactory* clone() const = 0;
    std::auto_ptr<Interpolator<Dim> > make_interpolator() const
        { return std::auto_ptr<Interpolator<Dim> >( make_interpolator_() ); }
};

}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR( dStorm::density_map::InterpolatorFactory<3> )
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR( dStorm::density_map::InterpolatorFactory<2> )

#endif
