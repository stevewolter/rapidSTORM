#ifndef DSTORM_FILLHOLESMOOTHING_H
#define DSTORM_FILLHOLESMOOTHING_H

#include <dStorm/engine/Image.h>

namespace dStorm {
namespace spotFinders {

std::auto_ptr< engine::spot_finder::Factory >
    make_fillhole_smoother();

}
}

#endif
