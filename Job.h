#ifndef DSTORM_JOB_H
#define DSTORM_JOB_H

#include <simparm/NodeHandle.h>

namespace dStorm {

struct Job {
    virtual ~Job() {}
    virtual void run() = 0;
    virtual simparm::NodeHandle attach_ui( simparm::NodeHandle ) = 0;
    virtual void stop() = 0;
    virtual void close_when_finished() = 0;
};

}

#endif
