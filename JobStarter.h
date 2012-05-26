#ifndef DSTORM_JOBSTARTER_H
#define DSTORM_JOBSTARTER_H

#include <simparm/TriggerEntry.h>
#include <dStorm/Config.h>
#include "job/Config.h"
#include <dStorm/stack_realign.h>
#include "MainThread.h"

namespace dStorm {

namespace job { struct Car; }

class JobStarter
: public simparm::TriggerEntry
{
    MainThread* master;
    job::Config& config;
    simparm::NodeHandle attachment_point;

    simparm::BaseAttribute::ConnectionStore listening;

    void start_job();
    DSTORM_REALIGN_STACK static void run_job( boost::shared_ptr<job::Car>, MainThread* );
  public:
    JobStarter( MainThread*, simparm::NodeHandle attachment_point, job::Config& config );
    void attach_ui( simparm::NodeHandle );
};

}

#endif
