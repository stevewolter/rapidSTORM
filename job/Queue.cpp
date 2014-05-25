#include "job/Queue.h"
#include "debug.h"

namespace dStorm {
namespace job {

Queue::Queue( frame_index i, int producer_count ) 
: next_output(i.value()), producer_count(producer_count), interruption( false )
{
    DEBUG("Starting queue " << this << " with first output " << next_output);
}

Queue::~Queue() {
    DEBUG("Destructing queue " << this);
}

void Queue::producer_finished()
{
    DEBUG("Noticing that producer has finished");
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    --producer_count; 
    DEBUG("Have now " << producer_count << " producers");
    if ( producer_count == 0 )
        consumer_can_continue.notify_all();
}

void Queue::notice_error( boost::exception_ptr e ) {
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    error = e;
}

void Queue::rethrow_exception() {
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    if ( error )
        boost::rethrow_exception(error);
}

void Queue::push( const output::LocalizedImage& r ) {
    boost::unique_lock<boost::mutex> lock(ring_buffer_mutex);
    DEBUG("Pushing image " << r.group << " into " << this);
    while ( r.group - next_output >= int(ring_buffer.size()) ) 
    {
        if ( interruption ) throw boost::thread_interrupted();
        DEBUG("Waiting for space for " << r.group);
        producer_can_continue.wait(lock);
    }

    DEBUG("Inserting image " << r.group << " into queue " << this);
    int ring = r.group % ring_buffer.size();
    assert( ! ring_buffer[ring].is_initialized() );
    ring_buffer[ring] = r;
    if ( r.group == next_output )
        consumer_can_continue.notify_all();

    if ( interruption ) throw boost::thread_interrupted();
}

bool Queue::has_more_input() {
    boost::unique_lock<boost::mutex> lock(ring_buffer_mutex);
    DEBUG("Pulling image " << ring());
    while ( ! ring_buffer[ring()].is_initialized() )
    {
        if ( producer_count == 0 ) return false;
        consumer_can_continue.wait( lock );
    }
    DEBUG("Pulled image " << ring());
    return true;
}

const output::LocalizedImage& Queue::front() const {
    DEBUG("Accessed " << ring());
    return *ring_buffer[ring()];
}

void Queue::pop() {
    DEBUG("Removed image " << ring());
    boost::lock_guard<boost::mutex> lock(ring_buffer_mutex);
    ring_buffer[ring()].reset();
    next_output += 1;
    producer_can_continue.notify_all();
}

void Queue::interrupt_producers() {
    boost::lock_guard<boost::mutex> lock(ring_buffer_mutex);
    DEBUG("Interrupting all producers for queue " << this);
    interruption = true;
    producer_can_continue.notify_all();
}

}
}
