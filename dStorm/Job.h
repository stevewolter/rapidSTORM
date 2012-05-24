#ifndef DSTORM_JOB_H
#define DSTORM_JOB_H

#include <simparm/NodeHandle.h>

namespace dStorm {

struct Job {
    virtual ~Job() {}
    virtual void attach_ui( simparm::NodeHandle ) = 0;
    virtual void detach_ui( simparm::NodeHandle ) = 0;
    virtual void stop() = 0;
    virtual bool needs_stopping() { return false; }
};

}

#endif
