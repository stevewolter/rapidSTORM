#include <boost/units/quantity.hpp>
#include <boost/units/cmath.hpp>

namespace Eigen {
    template <typename Unit, typename Scalar>
    boost::units::quantity<Unit, Scalar> ei_abs( const boost::units::quantity<Unit, Scalar>& a ) {
        return boost::units::abs(a);
    }
}

#include <Eigen/Core>
#include "Eigen_decl.hh"

namespace Eigen {
    template <typename Unit, typename Scalar>
    struct NumTraits< boost::units::quantity<Unit, Scalar> > : public NumTraits< Scalar > {};
    template <typename Unit, typename Scalar>
    struct NumTraits< const boost::units::quantity<Unit, Scalar> > : public NumTraits< Scalar > {};

}

#include "BoostUnits.hh"
#include "Eigen.hh"
#include "Entry.hh"
#include "Set.hh"
#include "Entry_Impl.hh"
#include "IO.hh"

#include <boost/units/systems/si/length.hpp>
#include <stdlib.h>

using namespace std;
using namespace simparm;

int main() {
    simparm::Entry< Eigen::Matrix<float, 2,2, Eigen::DontAlign> > float_matrix("FloatMatrix", "FloatMatrix");
    simparm::Entry< Eigen::Matrix< boost::units::quantity<boost::units::si::length,double>, 2,1, Eigen::DontAlign> > 
        meter_vector("MeterVector", "MeterVector");
    simparm::Entry< boost::units::quantity<boost::units::si::length,unsigned int> > bounded_entry("BoundedEntry", "Bounded entry");

    float_matrix = Eigen::Matrix<float, 2,2, Eigen::DontAlign>::Zero();
    simparm::Entry< boost::units::quantity<boost::units::si::length,double> > normal_entry("NormalEntry", "Normal entry");
    bounded_entry.value = 15 * boost::units::si::meter;
    bounded_entry.min = 5 * boost::units::si::meter;

    IO io(&cin, &cout);
    float_matrix.attach_ui(io);
    meter_vector.attach_ui(io);
    normal_entry.attach_ui(io);
    bounded_entry.attach_ui(io);
    while (true) {
        try {
            io.processInput();
            return EXIT_SUCCESS;
        } catch (const std::runtime_error& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}
