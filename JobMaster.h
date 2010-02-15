#ifndef DSTORM_JOB_MASTER_H
#define DSTORM_JOB_MASTER_H

#include <simparm/Node.hh>
#include "engine/Car_decl.h"

namespace dStorm {

class JobMaster {
  public:
    virtual ~JobMaster() {}
    
    virtual void register_node( engine::Car& ) = 0;
    virtual void erase_node( engine::Car&  ) = 0;

};

}

#endif
