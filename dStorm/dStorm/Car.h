#ifndef DSTORM_CAR_H
#define DSTORM_CAR_H

#include <cc++/thread.h>
#include <dStorm/CarConfig.h>
#include <dStorm/TransmissionConfig.h>
#include <cassert>

namespace dStorm {
    class Carburettor;
    class Engine;
    class Transmission;

    /** The Car class is the public frontend of the dStorm library.
     *  If supplied with a configuration, it can be used to construct
     *  the desired output elements and run (concurrently or not)
     *  the dStorm engine. */
    class Car : public ost::Thread {
      private:
        /** Construction Configuration. This is a copy of the CarConfig used
         *  to build this car. */
        CarConfig config;
        /** Runtime configuration. This is the storage locations for all
         *  configuration items which show job progress and status. */
        simparm::ConfigSet runtime_config;

        /** If no Carburettor was supplied, the automatically constructed
         *  default is stored here. */
        std::auto_ptr<Carburettor> myCarburettor;
        /** The input pointer is always set, either with the supplied
         *  input or an automatically constructed one. */
        Carburettor* input;

        /** If an Engine is needed, it is stored here. */
        std::auto_ptr<Engine> myEngine;

        /** If no Transmission was supplied, the automatically constructed
         *  default is stored here. */
        std::auto_ptr<Transmission> myTransmission;
        /** The output reference is always set, either with the supplied
         *  input or an automatically constructed one. */
        Transmission* output;

        /** Run the dStorm engine class. */
        void runWithEngine() throw();
        /** Simulate an engine run with the localisations supplied in a
         *  STM file. */
        void runOnSTM() throw();

      public:
        Car (const CarConfig &config, 
             Carburettor* input = NULL, 
             Transmission* output = NULL
          ) throw (std::exception) ;
        void keepCarburettor() throw();
        void keepTransmission() throw();

        void run() throw();
        virtual ~Car() throw();

        const CarConfig &getConfig() const throw() { return config; }

        /** Callback routine that is called just before a Car starts
         *  the engine. Can be used to add items to crankshaft. */
        static void (*carStartCallback)
            (CarConfig& config,
             Carburettor &input,
             Transmission &output) throw();
    };
}
#endif
