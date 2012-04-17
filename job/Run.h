#ifndef DSTORM_JOB_RUN_H
#define DSTORM_JOB_RUN_H

#include "Queue.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <dStorm/Engine.h>
#include <dStorm/output/Output.h>
#include <dStorm/stack_realign.h>

namespace dStorm {
namespace job {

class Run
{
  public:
    typedef input::Source< output::LocalizedImage > Input;
    typedef output::Output Output;
    enum Result { Succeeded, Failed, Restart };

    Run( boost::mutex& mutex, frame_index first_image,
         Input& input, Output& output, int piston_count );
    Result run();
    ~Run();

    void interrupt();
    void restart();
    std::auto_ptr<EngineBlock> block();

  private:
    boost::mutex& mutex;
    Queue queue;
    boost::condition unblocked;
    boost::ptr_vector<boost::thread> threads;
    bool restarted, blocked;
    class Block;

    Input& input;
    Output& output;
    int piston_count;

    DSTORM_REALIGN_STACK void compute_input();
    void stop_computation();
};

}
}

#endif
