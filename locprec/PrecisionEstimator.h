#ifndef LOCPREC_PRECISIONESTIMATOR_H
#define LOCPREC_PRECISIONESTIMATOR_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace locprec {

std::auto_ptr< dStorm::output::OutputSource > make_precision_estimator_source();

}

#endif
