#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <nonlinfit/plane/fwd.h>
#include <boost/units/Eigen/Core>

namespace nonlinfit {
namespace plane {

/** This class is the base class for the tagged data classes. */
struct GenericData  {
    double pixel_size;
    Eigen::Matrix< double, 2, 1, Eigen::DontAlign> min, max;
};

}
}

#endif
