#ifndef DSTORM_JOB_RUN_H
#define DSTORM_JOB_RUN_H

#include "job/Queue.h"
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "base/Engine.h"
#include "output/Output.h"
#include "stack_realign.h"

namespace dStorm {
namespace job {

class Run
{
  public:
    typedef input::Source< output::LocalizedImage > Input;
    typedef output::Output Output;
    enum Result { Succeeded, Failed, Restart };

    Run( boost::recursive_mutex& mutex, frame_index first_image,
         Input& input, Output& output, int piston_count );
    Result run();
    ~Run();

    void interrupt();
    void restart();
    std::auto_ptr<EngineBlock> block();

  private:
    boost::recursive_mutex& mutex;
    Queue queue;
    boost::condition unblocked;
    boost::ptr_vector<boost::thread> threads;
    bool restarted, blocked;
    class Block;

    Input& input;
    Output& output;
    int piston_count;

    DSTORM_REALIGN_STACK void compute_input(int thread);
    void stop_computation();
};

}
}

#endif
