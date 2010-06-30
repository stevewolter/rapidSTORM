#include "System.h"
#include "CameraReference.h"
#include "StateMachine.h"
#include "Camera.h"
#include "SDK.h"
#include <cassert>
#include <time.h>
#include <pthread.h>
#include "debug.h"

#include <simparm/ChoiceEntry_Impl.hh>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace AndorCamera {

struct System::CamInfo {
    int usage_count;
    Camera* object;
    bool have_handle;
    CameraHandle handle;
    std::auto_ptr<StateMachine::Request> suppressor;

    CamInfo() : usage_count(0), object(NULL), have_handle(false) {}
    ~CamInfo() {
        if ( object != NULL ) delete object;
    }
};

class CameraChooser : public simparm::Node::Callback {
    System& system;
  public:
    CameraChooser(System& system) 
        : simparm::Node::Callback
            ( simparm::Event::ValueChanged),
        system(system) {}
    void operator()(const simparm::Event&) 
    {
        system.selectCamera( system.choice_entry->value()() );
    }
};

static pthread_mutex_t sPtrMutex = PTHREAD_MUTEX_INITIALIZER;
static System* singletonPtr = NULL;

System& System::singleton() {
    try {
        pthread_mutex_lock(&sPtrMutex);
        if ( singletonPtr == NULL )
            singletonPtr = new System();
        pthread_mutex_unlock(&sPtrMutex);
    } catch (...) {
        pthread_mutex_unlock(&sPtrMutex);
        throw;
    }
    return *singletonPtr;
}

void System::free() {
    try {
        pthread_mutex_lock(&sPtrMutex);
        if ( singletonPtr != NULL ) {
            delete singletonPtr;
            singletonPtr = NULL;
        }
        pthread_mutex_unlock(&sPtrMutex);
    } catch(...) {
        pthread_mutex_unlock(&sPtrMutex);
        throw;
    }
}

System::System() {
    currentCamera = -1;
    cams = NULL;
    try {
        number_of_cameras = SDK::GetAvailableCameras();
    } catch (const Error &e) {
        std::cerr << e.what() << "\n";
        number_of_cameras = 0;
    }
    cams = new CamInfo[number_of_cameras];

    choice_entry = new simparm::ChoiceEntry(
                "ChooseCamera","Choose camera to connect to");
    callback = new CameraChooser(*this);

    simparm::ChoiceEntry& chooseCamera = *choice_entry;
    chooseCamera.value.addChangeCallback( *callback );

    DEBUG("Found " << number_of_cameras << " cameras");
    for (char i = 0; i < number_of_cameras; i++)
        chooseCamera.addChoice
            ("Camera" + std::string(1, 'A'+i), 
                "Camera " + std::string(1, 'A'+i));
    chooseCamera.setViewable( (number_of_cameras > 1) );
    chooseCamera.setUserLevel(simparm::Object::Beginner);
}

System::~System() {
    for ( std::vector<Listener*>::iterator i = listeners.begin(); 
          i != listeners.end();i ++)
        (*i)->listening_to = NULL;

    if ( cams != NULL )
        delete[] cams;
}

void System::acquireCamera(int number) {
    assert( number < number_of_cameras );
    
    if ( cams[number].object == NULL )
        cams[number].object = new Camera(number);

    cams[number].usage_count ++;
}

void System::releaseCamera(int number) {
    assert( number < number_of_cameras );
    
    cams[number].usage_count --;
    if ( cams[number].usage_count == 0 ) {
        if ( currentCamera == number ) {
            int prevCamera = currentCamera;
            currentCamera = -1;
            notifyListeners( prevCamera, currentCamera );
        }

        delete cams[number].object;
        cams[number].object = NULL;

    }
}

void System::selectCamera(int number) {
    if ( number == currentCamera ) return;
    if ( currentCamera != -1 ) {
        cams[currentCamera].suppressor
            = cams[currentCamera].object->state_machine()
                .ensure_at_most(States::Connected, StateMachine::User);
        cams[currentCamera].suppressor->wait_for_fulfillment();
    }

    if ( ! cams[number].have_handle ) {
        cams[number].handle = SDK::GetCameraHandle(number);
        cams[number].have_handle = true;
    }
    SDK::SetCurrentCamera( cams[number].handle );

    int prevCamera = currentCamera;
    currentCamera = number;

    if ( number != -1 )
        cams[currentCamera].suppressor.reset( NULL );
    notifyListeners( prevCamera, currentCamera );
}

/* See AndorCamera/Control.h for documentation */
void System::sleep(const unsigned int milliseconds) {
#ifdef HAVE_USLEEP
    usleep( 1000 * milliseconds );
#else
#ifdef HAVE_WINDOWS_H
    Sleep( milliseconds );
#endif
#endif
}

void System::add_listener( System::Listener &l ) 
{ 
    if ( currentCamera != -1 )
        l.current_camera_changed( -1, currentCamera );
    listeners.push_back(&l); 
    l.listening_to = this;
}
void System::remove_listener( System::Listener &l ) { 
    for ( std::vector<Listener*>::iterator i =
                listeners.begin(); i != listeners.end(); i++)
        if ( &l == *i ) {
            listeners.erase( i );
            l.listening_to = NULL;
            break;
        }
}
void System::notifyListeners(int from, int to) {
    for ( std::vector<Listener*>::iterator i = listeners.begin(); 
          i != listeners.end();i ++)
        (*i)->current_camera_changed( from, to );
}

simparm::ChoiceEntry& System::get_camera_chooser() 
{
    return *choice_entry;
}

System::Listener::~Listener() {
    if ( listening_to != NULL ) 
        listening_to->remove_listener(*this);
}

CameraReference System::get_current_camera() 
{
    if ( currentCamera == -1 )
        throw std::runtime_error("No camera selected");
    else
        return CameraReference(currentCamera);
}

CameraReference::CameraReference(int number) 
: num(number)
{
    System::singleton().acquireCamera(number);
}

CameraReference::CameraReference(const CameraReference &c) 
: num(c.num)
{
    System::singleton().acquireCamera(num);
}

CameraReference::~CameraReference() 
{
    System::singleton().releaseCamera(num);
}

Camera *CameraReference::getCamObject() const 
{
    return System::singleton().cams[num].object;
}

}
