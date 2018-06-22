#include "input_simulation/FluorophoreDistributions.h"
#include <boost/lexical_cast.hpp>
#include <boost/units/cmath.hpp>

namespace input_simulation {
namespace FluorophoreDistributions {

class Lattice : public FluorophoreDistribution {
  protected:
    void attach_ui( simparm::NodeHandle at )  {
        simparm::NodeHandle r = attach_parent(at);
        latticeDistance.attach_ui(r);
        border.attach_ui( r );
    }
    typedef Eigen::Matrix< 
        boost::units::quantity< boost::units::si::nanolength >,
        dStorm::samplepos::RowsAtCompileTime,
        dStorm::samplepos::ColsAtCompileTime,
        Eigen::DontAlign >
        NanoSamplePos;
  public:
    simparm::Entry< NanoSamplePos > latticeDistance, border;

    Lattice();
    Lattice* clone() const { return new Lattice(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};

Lattice::Lattice()
: FluorophoreDistribution("Lattice", "Fluorophores on square lattice"),
  latticeDistance("LatticeSpacing", "Distance between lattice points", 
    NanoSamplePos::Constant(40.0f * si::nanometre) ),
  border("LatticeBorder", "Distance between first/last lattice point "
                          " and image border in pixels", 
    NanoSamplePos::Constant(0.0f * si::nanometre) )
{
}

void iterate_dimension(
    FluorophoreDistribution::Positions::value_type& pos,
    int dim,
    FluorophoreDistribution::Positions& accum,
    const Fluorophore::Position& dist,
    const Fluorophore::Position& border,
    const FluorophoreDistribution::Size &size
) {
    FluorophoreDistribution::Positions::value_type::Scalar d;
    if ( 2.0f * border[dim] > size[dim] )
        throw std::runtime_error("Border for lattice distribution is too wide "
            "in dimension " + boost::lexical_cast<std::string>(dim) );
    for (d = border[dim]; d <= size[dim] - border[dim]; d += dist[dim]) 
    {
        pos[dim] = d;
        if ( dim == 0 ) {
            accum.push( pos );
        } else
            iterate_dimension(pos, dim-1, accum, dist, border, size);
    }
}

FluorophoreDistribution::Positions Lattice::fluorophore_positions(
    const FluorophoreDistribution::Size& size,
    gsl_rng*
) const {
    Positions rv;
    Positions::value_type pos;

    /** Ensure size & position are vectors of same size. */
    assert( pos.cols() == 1 && size.cols() == 1 );
    assert( pos.rows() == size.rows() );

    typedef FluorophoreDistribution::Positions::value_type::Scalar Length;
    iterate_dimension( pos, pos.rows()-1, rv, 
        latticeDistance().cast<Fluorophore::Position::Scalar>(), 
        border().cast<Fluorophore::Position::Scalar>(), size );

    return rv;
}

std::auto_ptr< FluorophoreDistribution >
make_lattice() { return std::auto_ptr<FluorophoreDistribution>(new Lattice()); }

}
}
