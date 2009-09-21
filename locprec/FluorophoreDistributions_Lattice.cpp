#include "FluorophoreDistributions.h"

namespace locprec {
namespace FluorophoreDistributions {

_Lattice::_Lattice()
: FluorophoreDistribution("Lattice", "Fluorophores on square lattice"),
  latticeDistance("LatticeSpacing", "Distance between lattice points"
                  " in pixels", 0.1),
  border("LatticeBorder", "Distance between first/last lattice point "
                          " and image border in pixels", 0.05)
{
}

void iterate_dimension(
    FluorophoreDistribution::Positions::value_type& pos,
    int dim,
    FluorophoreDistribution::Positions& accum,
    FluorophoreDistribution::Positions::value_type::Scalar dist,
    FluorophoreDistribution::Positions::value_type::Scalar border,
    const FluorophoreDistribution::Size &size
) {
    FluorophoreDistribution::Positions::value_type::Scalar d;
    for (d = border; d <= size[dim] - border; d += dist) {
        pos[dim] = d;
        if ( dim == 0 ) {
            accum.push( pos );
        } else
            iterate_dimension(pos, dim-1, accum, dist, border, size);
    }
}

FluorophoreDistribution::Positions _Lattice::fluorophore_positions(
    const Size& size,
    gsl_rng*
) {
    Positions rv;
    Positions::value_type pos;

    /** Ensure size & position are vectors of same size. */
    assert( pos.cols() == 1 && size.cols() == 1 );
    assert( pos.rows() == size.rows() );

    iterate_dimension( pos, pos.rows()-1, rv, latticeDistance(),
                                              border(), size );

    return rv;
}

}
}
