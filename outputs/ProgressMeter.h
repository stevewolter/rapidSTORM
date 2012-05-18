#ifndef DSTORM_PROGRESSMETER_H
#define DSTORM_PROGRESSMETER_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace dStorm {
namespace output {

std::auto_ptr< output::OutputSource > make_progress_meter_source();

}
}

#endif
