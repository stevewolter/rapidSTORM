#ifndef DSTORM_ENGINE_H
#define DSTORM_ENGINE_H

#include <memory>
#include <cc++/thread.h>
#include <data-c++/Vector.h>
#include "Config.h"

namespace dStorm {
   class Carburettor;
   class Config;
   class Transmission;

   /** The Engine class performs the main computation in the
    *  dSTORM module. It forks several threads (called pistons),
    *  which request images from the Carburettor, make the
    *  SpotFinder locate candidates in them, fit them with the
    *  SpotFitter and output the results to the Crankshaft. */
   class Engine : public simparm::ConfigSet {
      private:
        Config& config;

        friend class EngineThread;
        data_cpp::Vector< std::auto_ptr<ost::Thread> > pistons;

        Carburettor& input;
        Transmission* output;

        /** Flag set when computation threads should stop and be collected
         *  even though target images remain. */
        bool emergencyStop;
        /** Flag set when computation should be aborted and not restarted. */
        bool error;

        /** Run a piston and catch exceptions, setting the emergencyStop
          * and error flags. */
        void safeRunPiston() throw();
        /** The runPiston() method does the actual process controlling */
        void runPiston() throw(std::exception);

        /** Clean all structures, the crankshaft listeners and prepare
         *  engine restart. */
        void restart() throw();

        /** Start a new piston thread and save it in the piston list. */
        void addPiston() throw();
        void collectPistons() throw();

      public:
         Engine(Config& config, Carburettor& input);
         virtual ~Engine() throw();

         void setOutput( Transmission &newOutput ) throw() 
            { output = &newOutput; }
         /** Compute with the given number of subthreads, including
          *  the current one. */
         void run() throw();
         void stop() throw() { emergencyStop = error = true; }
   };
}

#endif
