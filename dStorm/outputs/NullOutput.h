#ifndef DSTORM_NULL_OUTPUT_H
#define DSTORM_NULL_OUTPUT_H

#include "../output/Output.h"

namespace dStorm {
namespace outputs {

struct NullOutput : public output::OutputObject 
{
    NullOutput();
    NullOutput* clone() const { return new NullOutput(); }
    AdditionalData announceStormSize(const Announcement&) 
        { return AdditionalData(); }
    void propagate_signal(ProgressSignal) {}
    Result receiveLocalizations(const EngineResult&)
        { return KeepRunning; }
};

}
}

#endif
