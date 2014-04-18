#ifndef LOCPREC_ROI_FILTER_H
#define LOCPREC_ROI_FILTER_H

#include <memory>
#include "output/OutputSource.h"

namespace dStorm {
namespace outputs {

std::auto_ptr< dStorm::output::OutputSource > make_roi_filter_source();

}
}

#endif
