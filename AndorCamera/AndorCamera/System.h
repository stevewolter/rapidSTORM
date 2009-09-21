#ifndef ANDORCAMERA_SYSTEM_H
#define ANDORCAMERA_SYSTEM_H

#include <AndorCamera/CameraReference.h>
#include <simparm/ChoiceEntry.hh>

#include <vector>

namespace AndorCamera {

typedef long CameraHandle;

class System {
  private:
    friend class Camera;
    friend class CameraReference;
    friend class CameraChooser;

    struct CamInfo;
    CamInfo* cams;

    int number_of_cameras, currentCamera;

    void acquireCamera(int index);
    void releaseCamera(int index);
    void notifyListeners(int from, int to);

    simparm::ChoiceEntry* choice_entry;
    simparm::Node::Callback* callback;

    System();
    ~System();

  public:
    /** Get a reference to the singleton class instance. */
    static System& singleton();
    /** Release the singleton class instance, 
     *  freeing all memory allocated. */
    static void free();

    int get_number_of_cameras() { return number_of_cameras; }
    void selectCamera(int index);
    CameraReference get_current_camera();

    simparm::ChoiceEntry& get_camera_chooser();

    /** \brief System-independent sleep() function.
        *
        *  \param milliseconds Number of ms to sleep
        **/
    static void sleep(const unsigned int milliseconds);

    class Listener {
        friend class System;
        AndorCamera::System* listening_to;
        Listener(const Listener& l);

      public:
        Listener() : listening_to(NULL) {}
        virtual ~Listener();
        virtual void current_camera_changed(int from, int to)
 = 0;
    };

    void add_listener(Listener &l);
    void remove_listener(Listener &l);

  private:
    std::vector<Listener*> listeners;
};

};

#endif
