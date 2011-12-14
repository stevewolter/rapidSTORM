#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include "Car_decl.h"

#include <dStorm/stack_realign.h>
#include <dStorm/Job.h>
#include <dStorm/Engine.h>
#include "config/Grand.h"
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
#include <boost/array.hpp>
#include <boost/exception_ptr.hpp>

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
        dStorm::GrandConfig config;
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

        boost::mutex ring_buffer_mutex;
        bool close_job, terminate_early, repeat_run, blocked;
        boost::recursive_mutex mutex;
        boost::condition producer_can_continue, consumer_can_continue;
        frame_index first_output, next_output;
        boost::array< boost::optional<output::LocalizedImage>, 64 > ring_buffer;
        int producer_count;
        boost::exception_ptr error;

        /** Receive the signal from closeJob. */
        void operator()(const simparm::Event&);

        void output_or_store( const output::LocalizedImage& output );
        bool have_output_threads() const;

        class Block;
        class ComputationThread;
        class ActiveProducer;
        boost::ptr_vector<ComputationThread> threads;
        boost::thread master_thread;

        void add_additional_outputs();

        void compute_until_terminated();
        void output_ring_buffer();
        void run_computation( std::auto_ptr<ActiveProducer>, bool& stop );

      public:
        Car (JobMaster*, const dStorm::GrandConfig &config) ;
        virtual ~Car();

        void drive();
        void stop();
        bool needs_stopping() { return true; }
        DSTORM_REALIGN_STACK void run() ;

        const dStorm::GrandConfig &getConfig() const { return config; }
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
