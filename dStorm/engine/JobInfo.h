#ifndef DSTORM_ENGINE_JOBINFO_H
#define DSTORM_ENGINE_JOBINFO_H

#include "Config.h"
#include "Image_decl.h"

namespace dStorm {
namespace engine {

struct JobInfo {
    const Config& config;
    const InputTraits& traits;

    JobInfo( const Config& c, const InputTraits& i )
        : config(c), traits(i) {}
};


}
}

#endif
