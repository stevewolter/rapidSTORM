#ifndef DSTORM_ENGINE_H
#define DSTORM_ENGINE_H

#include <memory>
#include <cc++/thread.h>
#include <data-c++/Vector.h>
#include <simparm/TriggerEntry.hh>
#include <dStorm/Config.h>

namespace dStorm {
   class Input;
   class Config;
   class Output;

   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Input, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Output. */
   class Engine : public simparm::Object, public simparm::Node::Callback {
      private:
        Config& config;
        simparm::TriggerEntry stopper;

        friend class EngineThread;
        data_cpp::Vector< std::auto_ptr<ost::Thread> > pistons;

        Input& input;
        Output* output;

        /** Flag set when computation threads should stop and be collected
         *  even though target images remain. */
        bool emergencyStop;
        /** Flag set when computation should be aborted and not restarted. */
        bool error;
        /** Global flag to be set when computation should  be aborted. */
        static bool globalStop;

        /** Run a piston and catch exceptions, setting the emergencyStop
          * and error flags. */
        void safeRunPiston();
        /** The runPiston() method does the actual process controlling */
        void runPiston();

        /** Clean all structures, the crankshaft listeners and prepare
         *  engine restart. */
        void restart();

        /** Start a new piston thread and save it in the piston list. */
        void addPiston();
        void collectPistons();

      public:
         Engine(Config& config, Input& input, Output& output);
         virtual ~Engine();

         /** Compute with the given number of subthreads, including
          *  the current one. */
         void run();
         void stop() { emergencyStop = error = true; }

         void operator()(simparm::Node&, Cause cause, simparm::Node *) {
            if ( cause == ValueChanged && stopper.triggered() ) {
                stopper.untrigger();
                stop();
            }
         }

         static void stopAllEngines() { globalStop = true; }
   };
}

#endif
