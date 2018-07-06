#ifndef DSTORM_JOBSTARTER_H
#define DSTORM_JOBSTARTER_H

#include "simparm/TriggerEntry.h"
#include "base/Config.h"
#include "stack_realign.h"

namespace dStorm {

class JobStarter
: public simparm::TriggerEntry
{
    JobConfig& config;
    simparm::NodeHandle attachment_point;

    simparm::BaseAttribute::ConnectionStore listening;

    void start_job();
    DSTORM_REALIGN_STACK static void run_job( boost::shared_ptr<Job> );
  public:
    JobStarter( simparm::NodeHandle attachment_point, JobConfig& config );
    void attach_ui( simparm::NodeHandle );
    void set_attachment_point( simparm::NodeHandle a ) { attachment_point = a; }
};

}

#endif
