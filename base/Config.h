#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include "input/fwd.h"
#include "engine/SpotFinder_decl.h"
#include "engine/SpotFitterFactory_decl.h"
#include "Job.h"
#include <memory>
#include "simparm/NodeHandle.h"

namespace dStorm {
namespace output { class OutputSource; }

class JobConfig : public Job {
  public:
    virtual ~JobConfig() {}
    virtual simparm::NodeHandle attach_ui( simparm::NodeHandle ) = 0;
    virtual void attach_children_ui( simparm::NodeHandle ) = 0;
    virtual std::auto_ptr< Job > make_job() = 0;
    virtual JobConfig* clone() const = 0;
    virtual void run() {}
    virtual void stop() {}
    virtual void close_when_finished() {}
};

class Config : public JobConfig
{
  public:
    virtual ~Config() {}
    virtual void add_output( std::auto_ptr<output::OutputSource> ) = 0;
    void add_output( output::OutputSource* s ) 
        { add_output( std::auto_ptr<output::OutputSource>(s) ); }
};

}

#endif
