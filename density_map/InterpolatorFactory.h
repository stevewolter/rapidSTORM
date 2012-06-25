#ifndef DSTORM_DENSITY_MAP_INTERPOLATOR_FACTORY_H
#define DSTORM_DENSITY_MAP_INTERPOLATOR_FACTORY_H

#include <simparm/Choice.h>

namespace dStorm {
namespace density_map {

template <int Dim>
class InterpolatorFactory : public simparm::Choice {
    virtual Interpolator<Dim>* make_interpolator_() const = 0;
public:
    std::auto_ptr<Interpolator<Dim> > make_interpolator() const
        { return std::auto_ptr<Interpolator<Dim> >( make_interpolator_() ); }
};

}
}

#endif
