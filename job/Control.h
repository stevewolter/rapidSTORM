#ifndef DSTORM_JOB_USERCONTROL_H
#define DSTORM_JOB_USERCONTROL_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include "base/Engine.h"
#include "simparm/TriggerEntry.h"

namespace dStorm {
namespace job {

class Run;

class Control 
: public dStorm::Engine
{
    mutable boost::mutex mutex;
    bool close_job, abort_job;
    simparm::TriggerEntry abortJob;
    boost::condition allow_termination;
    int active_termination_blocks;

    boost::shared_ptr<Run> current_run;
    std::auto_ptr< input::BaseTraits > next_run_traits;

    void do_close_job();
    void do_abort_job();

    simparm::BaseAttribute::ConnectionStore listening[2];
    class TerminationBlock;

    std::auto_ptr<EngineBlock> block_termination();

public:
    Control( bool auto_terminate );
    void registerNamedEntries( simparm::NodeHandle );
    void set_current_run( boost::shared_ptr<Run> );
    void wait_until_termination_is_allowed();
    void stop();
    void close_when_finished() {
        boost::unique_lock<boost::mutex> lock( mutex );
        close_job = true;
        allow_termination.notify_all();
    }

    void restart();
    void repeat_results();
    bool can_repeat_results();
    void change_input_traits( std::auto_ptr< input::BaseTraits > );
    std::auto_ptr<EngineBlock> block();

    bool aborted_by_user() const;
    bool continue_computing() const;
    bool traits_changed() const { return next_run_traits.get(); }
    std::auto_ptr< input::BaseTraits > new_traits() { return next_run_traits; }
};

}
}

#endif
