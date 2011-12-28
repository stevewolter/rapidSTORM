#ifndef DSTORM_JOB_COMPUTATION_THREAD_H
#define DSTORM_JOB_COMPUTATION_THREAD_H

#include "debug.h"
#include "Queue.h"
#include <dStorm/engine/Input.h>
#include <dStorm/stack_realign.h>
#include <boost/thread/thread.hpp>

namespace dStorm {
namespace job {

class ComputationThread : private boost::noncopyable {
  private:
    typedef input::Source< output::LocalizedImage > Input;
    Queue& queue;
    Input& input;
    boost::thread thread;

  public:
    ComputationThread(Queue &queue, Input& input) 
    : queue(queue), input(input)
    { 
        thread = boost::thread( &ComputationThread::run, this );
    }

    DSTORM_REALIGN_STACK void run() {
        try {
            std::copy( input.begin(), input.end(), 
                std::back_inserter( queue ) );
        } catch ( const boost::thread_interrupted& e ) {
            /* Terminate normally. */
        } catch ( const boost::exception& e ) {
            queue.notice_error( boost::current_exception() );
        } catch ( const std::runtime_error& e ) {
            queue.notice_error( boost::copy_exception(e) );
        } catch (...) {
            std::cerr << "Unknown error occured in result computation" << std::endl;
        }
        queue.producer_finished();
    }

    ~ComputationThread() {
        boost::this_thread::disable_interruption di;
        DEBUG("Interrupting thread");
        thread.interrupt();
        DEBUG("Joining thread");
        thread.join(); 
        DEBUG("Joined thread");
    }
};

}
}

#endif
