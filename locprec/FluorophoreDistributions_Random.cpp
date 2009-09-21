#include "FluorophoreDistributions.h"

namespace locprec {
namespace FluorophoreDistributions {

_Random::_Random()
: FluorophoreDistribution("Random", "Randomly distributed fluorophores"),
  fluorophoreNumber("FluorophoreNumber", "Number of "
                    "fluorophore in virtual sample", 100)
{
}

FluorophoreDistribution::Positions _Random::fluorophore_positions(
    const Size& size,
    gsl_rng* rng
) {
    Positions rv;
    for (unsigned int i = 0; i < fluorophoreNumber(); i++) {
        Fluorophore::Position pos;
        for (int d = 0; d < size.cols() && d < pos.cols(); d++)
            pos[d] = gsl_rng_uniform(rng) * size[d];
        rv.push(pos);
    }
    return rv;
}

}
}
