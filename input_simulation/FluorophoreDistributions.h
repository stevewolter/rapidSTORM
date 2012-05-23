#ifndef LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H
#define LOCPREC_FLUOROPHORE_DISTRIBUTIONS_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include "FluorophoreDistribution.h"
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/TriggerEntry.hh>
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
