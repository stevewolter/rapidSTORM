#include "debug.h"
#include "Control.h"
#include "Run.h"

namespace dStorm {
namespace job {

Control::Control( bool auto_terminate )
: simparm::Listener( simparm::Event::ValueChanged ),
  close_job( auto_terminate ),
  abort_job( false ),
  abortJob("StopComputation", "Stop computation"),
  closeJob("CloseJob", "Close job"),
  active_termination_blocks(0)
{
    closeJob.helpID = "#CloseJob";
    abortJob.helpID = "#StopEngine";

    receive_changes_from( abortJob.value );
    receive_changes_from( closeJob.value );
}

void Control::registerNamedEntries( simparm::Node& runtime_config )
{
    abortJob.attach_ui( runtime_config );
    closeJob.attach_ui( runtime_config );
}

void Control::wait_until_termination_is_allowed()
{
    boost::unique_lock<boost::mutex> lock( mutex );
    while ( ! close_job || active_termination_blocks > 0 )
        allow_termination.wait(lock);
}

void Control::stop() {
    boost::lock_guard<boost::mutex> lock( mutex );
    closeJob.editable = false;
    abortJob.editable = false;
    close_job = true;
    abort_job = true;
    if ( current_run ) current_run->interrupt();
    allow_termination.notify_all();
}

void Control::restart() {
    boost::lock_guard<boost::mutex> lock( mutex );
    assert( current_run );
    current_run->restart();
}

void Control::repeat_results() {
    throw std::logic_error("Cannot repeat results from dSTORM master job control");
}
bool Control::can_repeat_results() { return false; }

void Control::change_input_traits( std::auto_ptr< input::BaseTraits > new_traits )
{
    next_run_traits = new_traits;
}

std::auto_ptr<EngineBlock> Control::block() {
    assert( current_run );
    return current_run->block();
}

void Control::set_current_run( boost::shared_ptr<Run> r ) {
    boost::lock_guard<boost::mutex> lock( mutex );
    current_run = r;
    if ( r.get() && abort_job )
        r->interrupt();
}

void Control::operator()(const simparm::Event& e) {
    if ( &e.source == &closeJob.value && e.cause == simparm::Event::ValueChanged && closeJob.triggered() )
    {
        closeJob.untrigger();
        DEBUG("Job close button allows termination" );
        boost::lock_guard<boost::mutex> lock( mutex );
        closeJob.editable = false;
        close_job = true;
        abort_job = true;
        if ( current_run ) current_run->interrupt();
        allow_termination.notify_all();
    } else if ( &e.source == &abortJob.value && e.cause == simparm::Event::ValueChanged && abortJob.triggered() )
    {
        abortJob.untrigger();
        DEBUG("Abort job button pressed");
        boost::lock_guard<boost::mutex> lock( mutex );
        abortJob.editable = false;
        abort_job = true;
        if ( current_run ) current_run->interrupt();
    }
}

bool Control::aborted_by_user() const
{
    boost::lock_guard<boost::mutex> lock( mutex );
    return abort_job;
}

bool Control::continue_computing() const {
    boost::lock_guard<boost::mutex> lock( mutex );
    return ! abort_job;
}

class Control::TerminationBlock 
: public EngineBlock
{
    Control& c;
public:
    TerminationBlock( Control& c ) : c(c) {
        boost::lock_guard<boost::mutex> lock( c.mutex );
        ++c.active_termination_blocks;
    }

    ~TerminationBlock() {
        boost::lock_guard<boost::mutex> lock( c.mutex );
        --c.active_termination_blocks;
        if ( c.active_termination_blocks == 0 )
            c.allow_termination.notify_all();
    }
};

std::auto_ptr<EngineBlock> Control::block_termination()
{
    return std::auto_ptr<EngineBlock>( new TerminationBlock(*this) );
}

}
}
