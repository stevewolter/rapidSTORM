#ifndef DSTORM_NULL_OUTPUT_H
#define DSTORM_NULL_OUTPUT_H

#include "output/Output.h"

namespace dStorm {
namespace outputs {

struct NullOutput : public output::Output
{
    NullOutput() {}
    void announceStormSize(const Announcement&) OVERRIDE {}
    void receiveLocalizations(const EngineResult&) OVERRIDE {}
};

}
}

#endif
