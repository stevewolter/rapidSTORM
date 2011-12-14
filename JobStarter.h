#ifndef DSTORM_JOBSTARTER_H
#define DSTORM_JOBSTARTER_H

#include <simparm/TriggerEntry.hh>
#include <dStorm/Config.h>
#include "config/Grand.h"
#include <dStorm/JobMaster.h>

namespace dStorm {

class JobStarter
: public simparm::TriggerEntry,
  simparm::Listener
{
    JobMaster* master;
    const GrandConfig* config;

    void operator()( const simparm::Event& );
  public:
    JobStarter( JobMaster* );
    void setConfig( const GrandConfig& config ) 
        { this->config= &config; }
};

}

#endif
