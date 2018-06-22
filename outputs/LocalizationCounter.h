#ifndef DSTORM_LOCALIZATIONCOUNTER_H
#define DSTORM_LOCALIZATIONCOUNTER_H

#include "output/OutputSource.h"
#include <memory>

namespace dStorm {
namespace output {

std::auto_ptr< output::OutputSource > make_localization_counter_source();

}
}
#endif

