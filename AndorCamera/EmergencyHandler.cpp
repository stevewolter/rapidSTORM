#include "EmergencyHandler.h"
#include <sstream>
#include <dStorm/helpers/thread.h>
#include "SDK.h"
#include "System.h"
#include <sched.h>
#include <boost/utility.hpp>

namespace AndorCamera {

static std::string my_tag = "--AndorCamera:Shutdown";

typedef EmergencyHandler::Args Args;
typedef EmergencyHandler::Tag Tag;

Tag EmergencyHandler::get_tag(int on_camera_id) {
    Args args;
    args.push_back( my_tag );
    std::stringstream ss;
    ss << on_camera_id;
    args.push_back( ss.str() );
    return dStorm::ErrorHandler::make_tag( args );
}

class CameraHandler
: public ost::Thread, public dStorm::Job
{
    dStorm::JobMaster& master;
    int cam;

    static ost::Mutex mutex;
    static std::set<int> reconnected_cameras;

    struct Config : boost::noncopyable, public simparm::Set
    {
        simparm::DoubleEntry temperature;
        Config() 
            : simparm::Set("CameraStatus", "Camera shutdown status"),
              temperature("CameraTemperature", "Temperature", 0) 
            { temperature.viewable = false; push_back( temperature ); }
    };
    Config config;

  public:
    CameraHandler(dStorm::JobMaster& master, int cam_id)
        : ost::Thread("EmergencyCameraHandler"),
          master(master), cam( cam_id ) 
        {
            master.register_node( *this );
        }

    void run() throw() {
        try {
            bool take_action = check_camera();
            if ( take_action )
                shut_down();
        } catch ( const std::exception& e ) {
            std::cerr << "Error in camera handling: " << e.what() << std::endl;
        } catch (...) {}
        master.erase_node( *this );
    }
    void stop() {}
    simparm::Node& get_config() { return config; }

    bool check_camera() {
        ost::MutexLock lock(mutex);
        if ( reconnected_cameras.find(cam) == 
                reconnected_cameras.end() )
        {
            reconnected_cameras.insert( cam );
            return true;
        } else 
            return false;
    }

    void shut_down() {
            ost::MutexLock lock(mutex);
            SDK::CameraHandle handle = SDK::GetCameraHandle(cam);
            SDK::SetCurrentCamera(handle);
            try { SDK::Initialize("..\\share\\andor"); }
                catch (const Error&) {}
            try { SDK::AbortAcquisition(); SDK::WaitForAcquisition(); }
                catch (const Error&) {}
            try { SDK::CoolerOFF(); }
                catch (const Error&) {}
            try {
                std::pair<bool, float> tstate;
                do {
                    tstate = SDK::GetTemperatureF();
                    config.temperature = tstate.second;
                    config.temperature.viewable = true;
                    System::sleep(100);
                } while ( tstate.second < -20 );
            } catch (const Error&) {}
            try { SDK::ShutDown(); } catch ( const Error& ) {}
    }
};

ost::Mutex CameraHandler::mutex;
std::set<int> CameraHandler::reconnected_cameras;

void EmergencyHandler::do_emergency_cleanup(
    Args& args, dStorm::JobMaster& master) 
{
    if ( args.front() == my_tag )
    {
        args.pop_front();
        int cam_id = atoi( args.front().c_str() );
        args.pop_front();
        (new CameraHandler(master,cam_id))->detach();
    }
}

}
