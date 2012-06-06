#ifndef DSTORM_MAINTHREAD_H
#define DSTORM_MAINTHREAD_H

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <dStorm/Job.h>
#include <set>

namespace dStorm {

class MainThread 
{
public:
    virtual ~MainThread() {}
    virtual void run_all_jobs() = 0;
    virtual void register_job( Job& ) = 0;
    virtual void unregister_job( Job& ) = 0;
    virtual void register_unstopable_job() = 0;
    virtual void unregister_unstopable_job() = 0;
    virtual void terminate_running_jobs() = 0;
    virtual int count_jobs() = 0;
};

}

#endif
