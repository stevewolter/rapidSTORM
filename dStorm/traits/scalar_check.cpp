#include "scalar.h"
#include <boost/units/io.hpp>
#include "position.h"

using namespace dStorm::traits;

int main() {
    typedef Scalar<Position> Scalar;
    Position::ValueType p;
    p.x() = 10 * boost::units::si::meter;
    p.z() = -20 * boost::units::si::meter;
    std::list< Scalar > scalars = Scalar::all_scalars();

    if ( scalars.size() == 3 && scalars.front().value(p) == p.x() && scalars.back().value(p) == p.z() )
        return EXIT_SUCCESS;
    else {
        for (std::list<Scalar>::iterator i = scalars.begin(); i != scalars.end(); ++i)
            std::cout << "Scalar has address " << &i->value(p) << " and value " << i->value(p) << std::endl;
    }
}
