#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include "Queue.h"

#include <dStorm/stack_realign.h>
#include <dStorm/Job.h>
#include "Config.h"
#include <dStorm/output/OutputSource.h>
#include <cassert>
#include <simparm/TriggerEntry.hh>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/fwd.h>
#include <dStorm/JobMaster.h>
#include <boost/utility.hpp>
#include <set>
#include <setjmp.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include "Control.h"

namespace dStorm {
namespace output { class Output; }
namespace job { 

    class Run;

    /** The Car class is the public frontend of the dStorm library.
     *  If supplied with a configuration, it can be used to construct
     *  the desired output elements and run (concurrently or not)
     *  the dStorm engine. */
    class Car 
        : boost::noncopyable, public Job
    {
      private:
        std::set<std::string> used_output_filenames;

        std::auto_ptr<JobHandle> job_handle;
        /** Unique job identifier. */
        std::string ident;
        /** Runtime configuration. This is the storage locations for all
         *  configuration items which show job progress and status. */
        simparm::Set runtime_config;

        typedef input::Source< output::LocalizedImage > Input;

        std::auto_ptr<Input> input;
        Engine* upstream_engine;
        std::auto_ptr<output::Output> output;

        boost::recursive_mutex mutex;
        frame_index first_output;
        const int piston_count;

        boost::shared_ptr<Run> current_run;
        Control control;

        void output_or_store( const output::LocalizedImage& output );
        bool have_output_threads() const;

        boost::thread master_thread, computation_thread;
        DSTORM_REALIGN_STACK void compute() ;

      public:
        Car (JobMaster*, const job::Config &config) ;
        virtual ~Car();

        void drive();
        void stop();
        bool needs_stopping() { return true; }
        DSTORM_REALIGN_STACK void run() ;

        simparm::Node& get_config() { return runtime_config; }

    };
}
}
#endif
