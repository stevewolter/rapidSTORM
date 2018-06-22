#ifndef DSTORM_DENSITY_MAP_INTERPOLATOR_CHOICE_H
#define DSTORM_DENSITY_MAP_INTERPOLATOR_CHOICE_H

#include <simparm/ManagedChoiceEntry.h>
#include "density_map/InterpolatorFactory.h"

namespace dStorm {
namespace density_map {

template <int Dim>
class InterpolatorChoice {
    simparm::ManagedChoiceEntry< InterpolatorFactory<Dim> > choice;
public:
    InterpolatorChoice();
    void attach_ui( simparm::NodeHandle );
    std::auto_ptr< Interpolator<Dim> > make() const;
};

}
}

#endif
