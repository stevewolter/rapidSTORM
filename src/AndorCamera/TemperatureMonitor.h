/** \file TemperatureMonitor.h
 *
 *  Definition of a class for concurrent monitoring of CCD temperature. */
#ifndef ANDORCAMERA_TEMPERATUREMONITOR_H
#define ANDORCAMERA_TEMPERATUREMONITOR_H

#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <dStorm/helpers/thread.h>

namespace AndorCamera {
    /** This class concurrently monitors the camera CCD temperature.
     *  It will constantly update a provided field with the
     *  current CCD temperature and manage this field's viewability. 
     *  Temperature monitoring will be started
     *  at constructor call and stops when the destructor is called.
     *  */
    class TemperatureMonitor : private dStorm::Thread {
      private:
        /** Reference to the updated field. */
        simparm::DoubleEntry &realTemperature;
        /** Set to true by the destructor to trigger termination. */
        bool askForDestruction;
        /** The method that is called in the subthread. */
        virtual void run() throw();

      public:
        /** Constructor. Does immediately start a subthread and returns. */
        TemperatureMonitor (simparm::DoubleEntry &realTemperature);
        /** Destructor. Triggers and waits for subthread termination. */
        ~TemperatureMonitor ();
    };
}
#endif
