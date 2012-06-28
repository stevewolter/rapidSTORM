#ifndef DSTORM_VIEWER_BACKEND_H
#define DSTORM_VIEWER_BACKEND_H

#include <dStorm/output/Output.h>
#include <memory>
#include "ColourScheme.h"

namespace dStorm {
namespace viewer {

class Config;
class Status;

struct Backend
{
    virtual ~Backend() {}
    virtual output::Output& getForwardOutput() = 0;

    virtual std::auto_ptr<Backend> change_liveness( Status& ) = 0;

    virtual void save_image(std::string filename, const Config&) = 0;

    virtual void set_histogram_power(float power) = 0;
    virtual void set_top_cutoff(float fraction) = 0;
    virtual void set_job_name( const std::string& name ) = 0;

    static std::auto_ptr<Backend> create( std::auto_ptr< ColourScheme >, Status& );
};

struct NoOpBackend : public Backend
{
    struct Output : public output::Output {
        Output* clone() const { return new Output(*this); }
        const simparm::NodeHandle getNode() const { throw std::logic_error("Not implemented"); }
        simparm::NodeHandle getNode() { throw std::logic_error("Not implemented"); }
        AdditionalData announceStormSize(const Announcement&) { return AdditionalData(); }
        RunRequirements announce_run(const RunAnnouncement&) { return RunRequirements(); }
        void receiveLocalizations(const EngineResult&) {}
    };
    Output o;

    ~NoOpBackend() {}
    output::Output& getForwardOutput() { return o; }

    std::auto_ptr<Backend> change_liveness( Status& ) 
        { throw std::logic_error("Not implemented"); }

    void save_image(std::string , const Config&) 
        { throw std::logic_error("Not implemented"); }

    void set_histogram_power(float) 
        { throw std::logic_error("Not implemented"); }
    void set_top_cutoff(float) 
        { throw std::logic_error("Not implemented"); }
    void set_job_name( const std::string& ) 
        { throw std::logic_error("Not implemented"); }
};

}
}

#endif
