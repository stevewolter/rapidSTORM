#ifndef ANDORCAMERA_EMERGENCYHANDLER_H
#define ANDORCAMERA_EMERGENCYHANDLER_H

#include <dStorm/error_handler.h>
#include <dStorm/JobMaster.h>

namespace AndorCamera {

class EmergencyHandler {
  public:
    typedef dStorm::ErrorHandler::CleanupTag Tag;
    typedef dStorm::ErrorHandler::CleanupArgs Args;
    static Tag get_tag(int on_camera_id);

    static void do_emergency_cleanup(Args&, dStorm::JobMaster&);
};

}

#endif
