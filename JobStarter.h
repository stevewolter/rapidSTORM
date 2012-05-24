#ifndef DSTORM_JOBSTARTER_H
#define DSTORM_JOBSTARTER_H

#include <simparm/TriggerEntry.h>
#include <dStorm/Config.h>
#include "job/Config.h"
#include <dStorm/JobMaster.h>

namespace dStorm {

class JobStarter
: public simparm::TriggerEntry
{
    JobMaster* master;
    job::Config* config;

    simparm::BaseAttribute::ConnectionStore listening;

    void start_job();
  public:
    JobStarter( JobMaster* );
    void setConfig( job::Config& config ) 
        { this->config= &config; }
    void attach_ui( simparm::NodeHandle );
};

}

#endif
