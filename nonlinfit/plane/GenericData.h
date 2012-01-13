#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <nonlinfit/plane/fwd.h>
#include <boost/units/Eigen/Core>

namespace nonlinfit {
namespace plane {

/** This class is the base class for the tagged data classes. */
template <typename _LengthUnit>
struct GenericData  {
    typedef _LengthUnit LengthUnit;
    typedef typename boost::units::multiply_typeof_helper
        <LengthUnit,LengthUnit>::type AreaUnit;

    boost::units::quantity<AreaUnit> pixel_size;
    Eigen::Matrix< boost::units::quantity<LengthUnit>, 2, 1, Eigen::DontAlign> min, max;
};

}
}

#endif
