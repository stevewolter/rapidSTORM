#include "BoostUnits.hh"
#include "Entry_Impl.hh"
#include "IO.hh"
#include <boost/units/systems/si/length.hpp>
#include <boost/units/io.hpp>
#include <dejagnu.h>

int main() {
    simparm::Entry< boost::units::quantity<boost::units::si::length,float> >
      meters("Name", "Desc");

    boost::units::quantity<boost::units::si::length,float> some_meters
      ( 1.0f * boost::units::si::metre );

    meters = some_meters;
    std::cout << meters() << " " << some_meters << std::endl;
    TestState().testrun( meters() == some_meters, "Assignment of variable to unit entry works" );
    meters = 5 * boost::units::si::metre;
    TestState().testrun( meters() == 5 * boost::units::si::metre, "Assignment of constant to unit entry works" );

    boost::shared_ptr<simparm::IO> io( new simparm::IO(&std::cin, &std::cout) );
    meters.attach_ui( io );
    return 0;
}
