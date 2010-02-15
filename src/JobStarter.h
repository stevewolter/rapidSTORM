#ifndef DSTORM_JOBSTARTER_H
#define DSTORM_JOBSTARTER_H

#include <simparm/TriggerEntry.hh>
#include "engine/CarConfig.h"
#include "JobMaster.h"

namespace dStorm {

class JobStarter
: public simparm::TriggerEntry,
  simparm::Listener
{
    JobMaster& master;
    const engine::CarConfig& config;

    void operator()( const simparm::Event& );
  public:
    JobStarter( const engine::CarConfig&, JobMaster& );
};

}

#endif
