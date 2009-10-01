#define ANDORCAMERA_TEMPERATUREMONITOR_CPP
#include "TemperatureMonitor.h"
#include "AndorCamera/System.h"
#include "SDK.h"

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
                    realTemperature = tstate.second;
                    /* This time is more or less arbitrary. The detector
                    * refreshes the value at a way lower rate, but we
                    * have no idea when it does; thus, polling with 10 Hz
                    * seems safe. */
                    System::sleep(100);
                }
            } catch (const std::exception &e) {
                cerr << "Error in camera temperature supervision: "
                    << e.what() << endl;
            }
            realTemperature.setViewable(false);
        } catch (...) {
            std::cerr << "An unknown error happened while supervising "
                         "camera temperature. Sorry." << endl;
        }
    }

    /* See AndorCamera/TemperatureMonitor.h for documentation */
    TemperatureMonitor::TemperatureMonitor(simparm::DoubleEntry &rt)
 
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
        PROGRESS("Destructed temperature monitor");
    }
}
