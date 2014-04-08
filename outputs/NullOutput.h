#ifndef DSTORM_NULL_OUTPUT_H
#define DSTORM_NULL_OUTPUT_H

#include "output/Output.h"

namespace dStorm {
namespace outputs {

struct NullOutput : public output::Output
{
    NullOutput() {}
    AdditionalData announceStormSize(const Announcement&) 
        { return AdditionalData(); }
    void receiveLocalizations(const EngineResult&) {}
    void attach_ui( simparm::NodeHandle ) {}
};

}
}

#endif
