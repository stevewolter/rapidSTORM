#include <memory>
#include "output/OutputSource.h"

namespace dStorm {
namespace kalman_filter {

std::auto_ptr< dStorm::output::OutputSource > create();
std::auto_ptr< dStorm::output::OutputSource > create_drift_correction();

}
}
