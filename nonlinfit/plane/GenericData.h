#ifndef NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H
#define NONLINFIT_IMAGE_PLANE_EVALUATOR_GENERIC_DATA_H

#include <Eigen/Core>
#include <nonlinfit/plane/fwd.h>

namespace nonlinfit {
namespace plane {

/** This class is the base class for the tagged data classes. */
class GenericData  {
  public:
    double pixel_size;
    Eigen::Matrix< double, 2, 1, Eigen::DontAlign> min, max;
};

}
}

#endif
