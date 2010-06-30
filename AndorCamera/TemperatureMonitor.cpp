#define ANDORCAMERA_TEMPERATUREMONITOR_CPP
#include "TemperatureMonitor.h"
#include "AndorCamera/System.h"
#include "SDK.h"
#include <dStorm/helpers/exception.h>
#include "debug.h"

using namespace std;
using namespace SDK;

namespace AndorCamera {
    /* See AndorCamera/TemperatureMonitor.h for documentation */
    void TemperatureMonitor::run() throw() {
        try {
            realTemperature.setViewable(true);
            try {
                pair<bool, float> tstate;
                while ( !askForDestruction ) {
                    tstate = GetTemperatureF();
                    realTemperature = 
                        tstate.second * boost::units::celsius::degree;
                    /* This time is more or less arbitrary. The detector
                    * refreshes the value at a way lower rate, but we
                    * have no idea when it does; thus, polling with 10 Hz
                    * seems safe. */
                    System::sleep(100);
                }
            } catch (const dStorm::exception &e) {
                simparm::Message m( e.get_message("Error in camera temperature supervision") );
                realTemperature.send(m);
            } catch (const std::exception &e) {
                simparm::Message m( "Error in camera temperature supervision", e.what() );
                realTemperature.send(m);
            }
            realTemperature.setViewable(false);
        } catch (...) {
            simparm::Message m( "Error in camera temperature supervision", "Unknown error type encountered" );
            realTemperature.send(m);
        }
    }

    /* See AndorCamera/TemperatureMonitor.h for documentation */
    TemperatureMonitor::TemperatureMonitor(dStorm::FloatCelsiusEntry &rt)
    : ost::Thread("Camera temperature monitor"),
      realTemperature(rt),
      askForDestruction(false)
    {
        start();
    }

    /* See AndorCamera/TemperatureMonitor.h for documentation */
    TemperatureMonitor::~TemperatureMonitor() {
        askForDestruction = (true);
        join();
        DEBUG("Destructed temperature monitor");
    }
}
