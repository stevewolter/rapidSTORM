#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTION_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTION_H

#include "input_simulation/Fluorophore.h"
#include "simparm/ObjectChoice.h"
#include <gsl/gsl_rng.h>
#include <Eigen/Core>
#include <queue>

namespace input_simulation {

class FluorophoreDistribution : public simparm::ObjectChoice {
  public:
    typedef dStorm::samplepos Size;
    typedef std::queue< Fluorophore::Position,
                        std::deque<Fluorophore::Position,
                        Eigen::aligned_allocator<Fluorophore::Position> > >
        Positions;

    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const = 0;

    FluorophoreDistribution(
        const std::string& name, const std::string& desc)
        : simparm::ObjectChoice(name,desc) {}
    virtual FluorophoreDistribution* clone() const = 0;
};

}

#endif
