#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include "Car_decl.h"

#include <dStorm/Job.h>
#include <dStorm/helpers/thread.h>
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

namespace dStorm {
namespace output { class Output; }
namespace engine { 
    /** The Car class is the public frontend of the dStorm library.
     *  If supplied with a configuration, it can be used to construct
     *  the desired output elements and run (concurrently or not)
     *  the dStorm engine. */
    class Car 
        : boost::noncopyable, public Job,
          public ost::Thread, private simparm::Node::Callback 
    {
      private:
        std::set<std::string> used_output_filenames;

        JobMaster* input_stream;
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
        std::auto_ptr<output::Output> output;

        ost::Mutex terminationMutex;
        bool terminate, emergencyStop, error;
        ost::Condition terminationChanged;

        /** Receive the signal from closeJob. */
        void operator()(const simparm::Event&);

        /** Run the computation subthread. */
        void run() throw();
        void abnormal_termination(std::string) throw();

        class ComputationThread;
        boost::ptr_vector<ComputationThread> threads;

        void add_additional_outputs();

        void add_thread();
        void compute_until_terminated();
        void run_computation();

      public:
        Car (JobMaster*, const dStorm::Config &config) ;
        virtual ~Car();

        void drive();
        void stop();
        bool needs_stopping() { return true; }

        const dStorm::Config &getConfig() const { return config; }
        simparm::Node& get_config() { return runtime_config; }
    };
}
}
#endif
