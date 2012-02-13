#include "dejagnu.h"
#include "scalar.h"
#include <boost/units/io.hpp>
#include "position.h"

namespace dStorm {
namespace traits {

void check_scalar( TestState& state ) 
{
    typedef Scalar<Position> Scalar;
    Position::ValueType p;
    p.x() = 10 * boost::units::si::meter;
    p.z() = -20 * boost::units::si::meter;
    std::list< Scalar > scalars = Scalar::all_scalars();

    state ( scalars.size() == 3 && scalars.front().value(p) == p.x() && scalars.back().value(p) == p.z(),
            "Traits scalar class works" );
}

}
}
