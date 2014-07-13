#include <memory>
#include "output/OutputSource.h"

namespace dStorm {
namespace kalman_filter {
namespace drift_estimator {

std::auto_ptr< dStorm::output::OutputSource > create();

}
}
}
