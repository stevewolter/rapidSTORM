#ifndef LOCPREC_VARIANCEESTIMATOR_H
#define LOCPREC_VARIANCEESTIMATOR_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace variance_estimator {

std::auto_ptr<dStorm::output::OutputSource> make_output_source();

}

#endif

