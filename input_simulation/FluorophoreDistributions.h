#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H

#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include "FluorophoreDistribution.h"
#include <simparm/Entry.h>
#include <simparm/ChoiceEntry.h>
#include <dStorm/UnitEntries.h>

namespace input_simulation {
namespace FluorophoreDistributions {

using namespace boost::units;

class Random : public FluorophoreDistribution {
  protected:
    void attach_ui( simparm::NodeHandle at ) 
        { fluorophoreNumber.attach_ui( attach_parent( at ) ); }
  public:
    simparm::Entry<unsigned long> fluorophoreNumber;

    Random();
    Random* clone() const { return new Random(*this); }
    virtual Positions fluorophore_positions(
        const Size& size, gsl_rng* rng) const;
};

std::auto_ptr< FluorophoreDistribution > make_lattice();
std::auto_ptr< FluorophoreDistribution > make_lines();

}
}

#endif
