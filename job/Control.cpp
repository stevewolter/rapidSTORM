#include "debug.h"
#include "job/Control.h"
#include "job/Run.h"

namespace dStorm {
namespace job {

Control::Control( bool auto_terminate )
: close_job( auto_terminate ),
  abort_job( false ),
  abortJob("StopComputation", "Stop computation"),
  active_termination_blocks(0)
{
    abortJob.setHelpID( "#StopEngine" );
}

void Control::registerNamedEntries( simparm::NodeHandle runtime_config )
{
    abortJob.attach_ui( runtime_config );

    listening[0] = abortJob.value.notify_on_value_change( 
        boost::bind( &Control::do_abort_job, this ) );
}

void Control::wait_until_termination_is_allowed()
{
    boost::unique_lock<boost::mutex> lock( mutex );
    while ( ! close_job || active_termination_blocks > 0 )
        allow_termination.wait(lock);
}

void Control::stop() {
    boost::lock_guard<boost::mutex> lock( mutex );
    abortJob.freeze();
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

void Control::do_abort_job() {
    if ( abortJob.triggered() )
    {
        abortJob.untrigger();
        DEBUG("Abort job button pressed");
        boost::lock_guard<boost::mutex> lock( mutex );
        abortJob.freeze();
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
