#ifndef LOCPREC_VARIANCEESTIMATOR_H
#define LOCPREC_VARIANCEESTIMATOR_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace dStorm { 
namespace outputs {

std::auto_ptr<dStorm::output::OutputSource> make_variance_estimator_source();

}
}

#endif

