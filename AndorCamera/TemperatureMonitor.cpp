#define ANDORCAMERA_TEMPERATUREMONITOR_CPP
#include "TemperatureMonitor.h"
#include "AndorCamera/Control.h"
#include "SDK.h"

using namespace std;

namespace AndorCamera {
    /* See AndorCamera/TemperatureMonitor.h for documentation */
    void TemperatureMonitor::run() throw() {
        realTemperature.setViewable(true);
        try {
            pair<bool, float> tstate;
            while ( !askForDestruction ) {
                tstate = GetTemperatureF();
                realTemperature = tstate.second;
                /* This time is more or less arbitrary. The detector
                 * refreshes the value at a way lower rate, but we
                 * have no idea when it does; thus, polling with 10 Hz
                 * seems safe. */
                Control::sleep(100);
            }
        } catch (const std::exception &e) {
            cerr << "Error in camera temperature supervision: "
                 << e.what() << endl;
        }
        realTemperature.setViewable(false);
    }

    /* See AndorCamera/TemperatureMonitor.h for documentation */
    TemperatureMonitor::TemperatureMonitor
        (ConfigEntryDouble &rt) throw () 
    : ost::Thread("Camera temperature monitor"),
      realTemperature(rt),
      askForDestruction(false)
    {
        start();
    }

    /* See AndorCamera/TemperatureMonitor.h for documentation */
    TemperatureMonitor::~TemperatureMonitor() throw() {
        askForDestruction = (true);
        join();
        PROGRESS("Destructed temperature monitor");
    }
}
