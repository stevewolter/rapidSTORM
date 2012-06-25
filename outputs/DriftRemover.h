#ifndef DSTORM_TRANSMISSIONS_DRIFT_REMOVER_H
#define DSTORM_TRANSMISSIONS_DRIFT_REMOVER_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace dStorm {
namespace drift_remover {

std::auto_ptr< output::OutputSource > make();

}
}

#endif
