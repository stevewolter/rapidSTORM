#ifndef LOCPREC_VARIANCEESTIMATOR_H
#define LOCPREC_VARIANCEESTIMATOR_H

#include <dStorm/output/OutputSource.h>

namespace variance_estimator {

class Output;

}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<variance_estimator::Output>();

}
}

#endif

