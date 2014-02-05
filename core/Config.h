#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <dStorm/input/fwd.h>
#include <dStorm/engine/SpotFinder_decl.h>
#include <dStorm/engine/SpotFitterFactory_decl.h>
#include "dStorm/InsertionPlace.h"
#include "dStorm/Job.h"
#include <memory>
#include <simparm/NodeHandle.h>

namespace dStorm {
namespace output { class OutputSource; }

struct JobConfig : public Job {
    virtual ~JobConfig() {}
    virtual simparm::NodeHandle attach_ui( simparm::NodeHandle ) = 0;
    virtual void attach_children_ui( simparm::NodeHandle ) = 0;
    virtual std::auto_ptr< Job > make_job() = 0;
    virtual JobConfig* clone() const = 0;
    virtual void run() {}
    virtual void stop() {}
    virtual void close_when_finished() {}
};

struct Config : public JobConfig
{
    virtual ~Config() {}
    virtual void add_input( std::auto_ptr<input::Link>, InsertionPlace ) = 0;
    void add_input( input::Link* s, InsertionPlace p ) 
        { add_input( std::auto_ptr<input::Link>(s), p ); }
    virtual void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> ) = 0;
    void add_spot_finder( engine::spot_finder::Factory* f ) 
            { add_spot_finder( std::auto_ptr<engine::spot_finder::Factory>(f) ); }
    virtual void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> ) = 0;
    void add_spot_fitter( engine::spot_fitter::Factory* f ) 
            { add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory>(f) ); }
    virtual void add_output( std::auto_ptr<output::OutputSource> ) = 0;
    void add_output( output::OutputSource* s ) 
        { add_output( std::auto_ptr<output::OutputSource>(s) ); }
};

}

#endif
