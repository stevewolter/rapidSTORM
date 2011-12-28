#include "Queue.h"
#include "debug.h"

namespace dStorm {
namespace job {

Queue::Queue( frame_index i, int producer_count ) 
: next_output(i), producer_count(producer_count)
{
    DEBUG("Starting queue with first output " << next_output);
}

Queue::~Queue() {
    DEBUG("Destructing queue");
}

void Queue::producer_finished()
{
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    --producer_count; 
    if ( producer_count == 0 )
        consumer_can_continue.notify_all();
}

void Queue::notice_error( boost::exception_ptr e ) {
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    error = boost::current_exception();
}

void Queue::rethrow_exception() {
    boost::lock_guard<boost::mutex> lock2( ring_buffer_mutex );
    if ( error )
        boost::rethrow_exception(error);
}

void Queue::push( const output::LocalizedImage& r ) {
    boost::unique_lock<boost::mutex> lock(ring_buffer_mutex);
    DEBUG("Pushing image " << r.forImage);
    while ( (r.forImage - next_output).value() >=
            int(ring_buffer.size()) ) 
        producer_can_continue.wait(lock);

    int ring = r.forImage.value() % ring_buffer.size();
    assert( ! ring_buffer[ring].is_initialized() );
    ring_buffer[ring] = r;
    if ( r.forImage == next_output )
        consumer_can_continue.notify_all();

    boost::this_thread::interruption_point();
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
    return *ring_buffer[ring()];
}

void Queue::pop() {
    boost::lock_guard<boost::mutex> lock(ring_buffer_mutex);
    ring_buffer[ring()].reset();
    next_output += 1 * camera::frame;
    producer_can_continue.notify_all();
}

}
}
