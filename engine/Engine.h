#ifndef DSTORM_ENGINE_H
#define DSTORM_ENGINE_H

#include <memory>
#include <dStorm/helpers/thread.h>
#include <dStorm/data-c++/Vector.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/engine/Config.h>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/output/Traits_decl.h>
#include <boost/utility.hpp>

namespace dStorm {
namespace output { class Output; }
namespace engine {
   class Config;

   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Input, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Output. */
   class Engine : public simparm::Object, public simparm::Node::Callback,
                  boost::noncopyable 
   {
      private:
        Config& config;
        simparm::TriggerEntry stopper;
        ost::Mutex mutex;
        simparm::UnsignedLongEntry errors;

        friend class EngineThread;
        data_cpp::Vector< std::auto_ptr<ost::Thread> > pistons;

        Input& input;
        output::Output* output;

        /** Flag set when computation threads should stop and be collected
         *  even though target images remain. */
        bool emergencyStop;
        /** Flag set when computation should be aborted and not restarted. */
        bool error;
        /** Global flag to be set when computation should  be aborted. */
        static bool globalStop;

        /** Run a piston and catch exceptions, setting the emergencyStop
          * and error flags. */
        void safeRunPiston() throw();
        /** The runPiston() method does the actual process controlling */
        void runPiston();

        /** Clean all structures, the crankshaft listeners and prepare
         *  engine restart. */
        void restart();

        /** Start a new piston thread and save it in the piston list. */
        void addPiston();
        void collectPistons();

        /** Compute changes to traits from input image to localizations
         *  output. */
        output::Traits convert_traits( const InputTraits& );

      public:
         Engine(Config& config, Input& input, output::Output& output);
         virtual ~Engine();

         /** Compute with the given number of subthreads, including
          *  the current one. */
         void run();
         void stop() { emergencyStop = error = true; }

         void operator()(simparm::Node&, Cause, simparm::Node *) {
            if ( stopper.triggered() ) {
                stopper.untrigger();
                stop();
            }
         }

         static void stopAllEngines() { globalStop = true; }
   };
}
}

#endif
