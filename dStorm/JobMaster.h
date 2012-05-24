#ifndef DSTORM_JOB_MASTER_H
#define DSTORM_JOB_MASTER_H

#include "Job.h"

namespace dStorm {

struct JobHandle {
    virtual ~JobHandle();
    virtual void unregister_node() = 0;
};

struct JobMaster {
    JobMaster();
    virtual ~JobMaster();
    
    virtual std::auto_ptr<JobHandle> register_node( Job& ) = 0;
};

}

#endif
