#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTION_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTION_H

#include "Fluorophore.h"
#include <simparm/Object.hh>
#include <gsl/gsl_rng.h>
#include <Eigen/Core>
#include <queue>

namespace locprec {

class FluorophoreDistribution : public simparm::Object {
  public:
    typedef dStorm::traits::Optics<2>::SamplePosition Size;
    typedef std::queue< Fluorophore::Position,
                        std::deque<Fluorophore::Position,
                        Eigen::aligned_allocator<Fluorophore::Position> > >
        Positions;

    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const = 0;

    FluorophoreDistribution(
        const std::string& name, const std::string& desc)
        : simparm::Object(name,desc) {}
    FluorophoreDistribution* clone() const = 0;
};

}

#endif
