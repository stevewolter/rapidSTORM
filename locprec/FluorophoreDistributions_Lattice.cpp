#include "FluorophoreDistributions.h"

namespace locprec {
namespace FluorophoreDistributions {

_Lattice::_Lattice()
: FluorophoreDistribution("Lattice", "Fluorophores on square lattice"),
  latticeDistance("LatticeSpacing", "Distance between lattice points", 40 * si::nanometre ),
  border("LatticeBorder", "Distance between first/last lattice point "
                          " and image border in pixels", 10 * si::nanometre )
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
    for (d = border; d <= size[dim] - border; d += dist) 
    {
        pos[dim] = d;
        if ( dim == 0 ) {
            accum.push( pos );
        } else
            iterate_dimension(pos, dim-1, accum, dist, border, size);
    }
}

FluorophoreDistribution::Positions _Lattice::fluorophore_positions(
    const FluorophoreDistribution::Size& size,
    gsl_rng*
) const {
    Positions rv;
    Positions::value_type pos;

    /** Ensure size & position are vectors of same size. */
    assert( pos.cols() == 1 && size.cols() == 1 );
    assert( pos.rows() == size.rows() );

    typedef FluorophoreDistribution::Positions::value_type::Scalar Length;
    iterate_dimension( pos, pos.rows()-1, rv, Length(latticeDistance()), Length(border()), size );

    return rv;
}

}
}
