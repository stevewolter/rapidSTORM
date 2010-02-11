#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include <dStorm/helpers/thread.h>
#include "CarConfig.h"
#include <dStorm/output/OutputSource.h>
#include <cassert>
#include <simparm/TriggerEntry.hh>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/Source_decl.h>
#include "MasterConfig.h"
#include <boost/utility.hpp>
#include <set>
#include <setjmp.h>

namespace dStorm {
namespace output { class Output; }
namespace engine { 
    class Engine;

    /** The Car class is the public frontend of the dStorm library.
     *  If supplied with a configuration, it can be used to construct
     *  the desired output elements and run (concurrently or not)
     *  the dStorm engine. */
    class Car 
        : boost::noncopyable,
          public ost::Thread, private simparm::Node::Callback 
    {
      private:
        std::set<std::string> used_output_filenames;

        /** Reference to the master configuration. Must be destructed last
         *  to ensure that all configuration items are destroyed before the
         *  DLLs are unloaded. */
        MasterConfig::Ptr master;

        /** Construction Configuration. This is a copy of the CarConfig used
         *  to build this car. */
        CarConfig config;
        /** Unique job identifier. */
        std::string ident;
        /** Runtime configuration. This is the storage locations for all
         *  configuration items which show job progress and status. */
        simparm::Set runtime_config;
        /** Button to close the job. */
        simparm::TriggerEntry closeJob;

        std::auto_ptr<input::BaseSource> source;
        std::auto_ptr<engine::Input> input;

        /** If we process ready-made localizations, their source is
         *  stored here. */
        std::auto_ptr< input::Source<Localization> > locSource;
        /** If an Engine is needed, it is stored here. */
        std::auto_ptr<engine::Engine> myEngine;

        std::auto_ptr<output::Output> output;

        static ost::Mutex terminationMutex;
        bool terminate;
        static bool terminateAll;
        static ost::Condition terminationChanged;

        void make_input_driver();
        /** Run the dStorm engine class. */
        void runWithEngine();
        /** Simulate an engine run with the localisations supplied in a
         *  STM file. */
        void runOnSTM() throw( std::exception );

        /** Receive the signal from closeJob. */
        void operator()(const simparm::Event&);

        /** Run the computation subthread. */
        void run() throw();
        void abnormal_termination(std::string) throw();

        bool panic_point_set;
        jmp_buf panic_point;

      public:
        Car (const MasterConfig::Ptr&, const CarConfig &config) ;
        virtual ~Car();

        void drive();
        static void terminate_all_Car_threads();

        const CarConfig &getConfig() const { return config; }
    };
}
}
#endif
