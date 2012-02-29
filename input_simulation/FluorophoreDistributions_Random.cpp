#include <boost/units/io.hpp>
#include "FluorophoreDistributions.h"

namespace input_simulation {
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
) const {
    Positions rv;
    for (unsigned int i = 0; i < fluorophoreNumber(); i++) {
        dStorm::samplepos pos;
        for (int d = 0; d < size.rows() && d < pos.rows(); d++)
            pos[d] = float(gsl_rng_uniform(rng)) * size[d];
        rv.push( pos );
    }
    return rv;
}

}
}
