#include "job/Run.h"
#include "debug.h"
#include <boost/thread/thread.hpp>
#include <dStorm/engine/Input.h>
#include <boost/version.hpp>

namespace dStorm {
namespace job {

Run::Run( boost::recursive_mutex& mutex, frame_index first_image, 
          Input& input, Output& output, int piston_count ) 
: mutex(mutex), queue(first_image, piston_count),
  restarted(false), blocked(false),
  input(input), output(output), piston_count(piston_count)
{
    DEBUG("Created run " << this);
}

Run::~Run() {
    DEBUG("Destructing run " << this);
    boost::this_thread::disable_interruption di;
    stop_computation();
}

void Run::interrupt() {
    DEBUG("Interrupting run " << this);
    queue.interrupt_producers();
#if BOOST_VERSION >= 104800
    /* Avoid boost thread interruption errors on Debian in 1.46.1 */
#endif
}

void Run::stop_computation() {
    queue.interrupt_producers();
    DEBUG("Joining threads for queue " << this);
    std::for_each( threads.begin(), threads.end(), 
        std::mem_fun_ref( &boost::thread::join ) );
    DEBUG("Cleared threads for queue " << this);
}

Run::Result Run::run() {
    boost::unique_lock<boost::recursive_mutex> lock( mutex );

    Output::RunRequirements r = 
        output.announce_run(Output::RunAnnouncement());
    if ( ! r.test(Output::MayNeedRestart) )
        input.dispatch( Input::WillNeverRepeatAgain );

    input.set_thread_count(piston_count);

    lock.unlock();

    for (int i = 0; i < piston_count; ++i)
        threads.push_back( new boost::thread( &Run::compute_input, this, i ) );

    while ( queue.has_more_input() )
    {
        lock.lock();
        while ( blocked ) { unblocked.wait( lock ); }
        DEBUG("Checking for thread interruption");
        boost::this_thread::interruption_point();
        DEBUG("Delivering results");
        output.receiveLocalizations( queue.front() );
        lock.unlock();
        queue.pop();
        if ( restarted ) return Restart;
    }
    DEBUG("Collecting threads");
    stop_computation();
    DEBUG("Collected threads");

    output.run_finished( output::Output::RunFinished() );
    if ( restarted ) return Restart;

    queue.rethrow_exception();
    return Succeeded;
}

struct Run::Block : public EngineBlock {
    Run& m;
    Block( Run& m ) : m(m) {
        boost::unique_lock<boost::recursive_mutex> lock(m.mutex);
        DEBUG( "Block in place" );
        m.blocked = true;
    }

    ~Block() {
        DEBUG( "Waiting for lock to put block out of place" );
        boost::unique_lock<boost::recursive_mutex> lock(m.mutex);
        m.blocked = false;
        DEBUG( "Block out of place" );
        m.unblocked.notify_all();
    }
};

std::auto_ptr<EngineBlock> Run::block() {
    return std::auto_ptr<EngineBlock>( new Block(*this) );
}

void Run::restart() {
    restarted = true;
}

void Run::compute_input(int thread) {
    try {
        output::LocalizedImage image;
        while (input.GetNext(thread, &image)) {
            DEBUG("Processed image " << image.forImage);
            queue.push_back(image);
        }
    } catch ( const boost::thread_interrupted& e ) {
        /* Terminate normally. */
    } catch ( const boost::exception& e ) {
        queue.notice_error( boost::current_exception() );
    } catch ( const std::runtime_error& e ) {
        queue.notice_error( boost::copy_exception(e) );
    }
    queue.producer_finished();
}


}
}
