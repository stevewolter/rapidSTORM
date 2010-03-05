#ifndef DSTORM_JOB_MASTER_H
#define DSTORM_JOB_MASTER_H

#include <simparm/Node.hh>
#include <dStorm/Job.h>

namespace dStorm {

class JobMaster {
  public:
    JobMaster();
    virtual ~JobMaster();
    
    virtual void register_node( Job& ) = 0;
    virtual void erase_node( Job&  ) = 0;

};

}

#endif
