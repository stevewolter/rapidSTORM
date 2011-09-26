#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include "Car_decl.h"

#include <dStorm/stack_realign.h>
#include <dStorm/Job.h>
#include <dStorm/Engine.h>
#include <dStorm/Config.h>
#include <dStorm/output/OutputSource.h>
#include <cassert>
#include <simparm/TriggerEntry.hh>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/Source_decl.h>
#include <dStorm/JobMaster.h>
#include <boost/utility.hpp>
#include <set>
#include <setjmp.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

namespace dStorm {
namespace output { class Output; }
namespace engine { 
    /** The Car class is the public frontend of the dStorm library.
     *  If supplied with a configuration, it can be used to construct
     *  the desired output elements and run (concurrently or not)
     *  the dStorm engine. */
    class Car 
        : boost::noncopyable, public Job,
          private simparm::Node::Callback ,
          public dStorm::Engine
    {
      private:
        std::set<std::string> used_output_filenames;

        std::auto_ptr<JobHandle> job_handle;
        /** Construction Configuration. This is a copy of the Config used
         *  to build this car. */
        dStorm::Config config;
        /** Unique job identifier. */
        std::string ident;
        /** Runtime configuration. This is the storage locations for all
         *  configuration items which show job progress and status. */
        simparm::Set runtime_config;
        simparm::TriggerEntry abortJob;
        simparm::TriggerEntry closeJob;

        typedef input::Source< output::LocalizedImage > Input;

        std::auto_ptr<Input> input;
        Engine* upstream_engine;
        std::auto_ptr<output::Output> output;

        boost::recursive_mutex mutex;
        bool terminate, emergencyStop, error, finished, blocked;
        boost::condition terminationChanged, next_output_changed;
        frame_index first_output, next_output;

        /** Receive the signal from closeJob. */
        void operator()(const simparm::Event&);

        class Block;
        class ComputationThread;
        boost::ptr_vector<ComputationThread> threads;
        boost::thread master_thread;

        void add_additional_outputs();

        void add_thread();
        void compute_until_terminated();
        DSTORM_REALIGN_STACK void run_computation();

      public:
        Car (JobMaster*, const dStorm::Config &config) ;
        virtual ~Car();

        void drive();
        void stop();
        bool needs_stopping() { return true; }
        DSTORM_REALIGN_STACK void run() ;

        const dStorm::Config &getConfig() const { return config; }
        simparm::Node& get_config() { return runtime_config; }

        void restart();
        void repeat_results();
        bool can_repeat_results();
        void change_input_traits( std::auto_ptr< input::BaseTraits > );
        std::auto_ptr<EngineBlock> block();
    };
}
}
#endif
