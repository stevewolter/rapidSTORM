/** \file Control.h
 *        This file contains the definition of the Control class */

#ifndef ANDORCAMERA_CONTROL_H
#define ANDORCAMERA_CONTROL_H

#include <ConfigEventHandler.hh>
#include <AndorCamera/ControlReference.h>
#include <set>
#include <stdint.h>
#include <cc++/thread.h>

#include <AndorCamera/Config.h>

namespace AndorCamera {
    class Acquisition;
    class TemperatureMonitor;

    /** \brief Singleton master class to control all Andor cameras. 
     *
     *  This singleton class is responsible for initializing and cooling
     *  a camera. It manages the \c Acquisition classes that are concurrently
     *  trying to access the camera and offers a range of control elements
     *  that allow supervision and control of camera operation.
     *
     * \sa AndorCamera::ControlReference */
    class Control : private ConfigEventHandler, public Config {
      public:
        /** Shorthand struct combining width and height. */
        struct Dimension { short w /** <width */, h /** <height */; };

        /** The directory to read camera ini files from. */
        ConfigEntryFile         configDir;
        /** If multiple cameras are installed, this field allows
            *  choosing from them. Has one choice for each camera. */
        ConfigEntryChoice       chooseCamera;

        /** /brief Controls and displays the cooling of the
            *  camera
            *
            *  If this element is set to true externally, the camera
            *  goes to the Cooling state; if it is set to false,
            *  to the Initialized state. If the camera goes to one
            *  of these states through program-internal processes 
            *  (for example, automatic acquisition preparation),
            *  doCool is updated to match the state. */
        ConfigEntryBool         doCool;
        /** The A/D converter channel that will be used by 
            *  acquisitions. */
        ConfigEntryChoice       adChannel;
        /** The desired exposure time for each image. */
        ConfigEntryDouble       exposureTime;
        /** The temperature that is required to be reached before the 
            *  shutter may be opened and acquisitions started. */
        ConfigEntryLong         requiredTemperature;
        /** The current CCD temperature. Is only updated while the 
            *  system is cooling. */
        ConfigEntryDouble       realTemperature;
        /** The vertical scan speed closest to this one will 
            *  be selected by default. */
        ConfigEntryDouble       desired_VS_Speed;
        /** The horizontal scan speed closest to this one will 
            *  be selected by default. */
        ConfigEntryDouble       desired_HS_Speed;
        /** The A/D converter channel with this depth will be selected
            *  by default. */
        ConfigEntryUnsignedLong adChannelDepth;

        /** Triggering this item makes the system connect to the chosen
            *  camera and initialize the fields below. */
        ConfigEntryTrigger      connect;
        /** Triggering this item makes the system connect to the chosen
            *  camera and initialize the fields below. */
        ConfigEntryTrigger      disconnect;

        /** Divider to separate connection from cooling */
        ConfigEntryDivider      preCool;

        /** Register all of the above-mentioned public configuration
            *  items.
            *
            *  \param cs Register at this ConfigSet
            **/
        void registerEntries(ConfigSet &cs) throw();

        /** If the camera is at least in the Initialized state,
            *  this function returns the detector (that is, CCD) 
            *  size in pixels. */
        Dimension getDetectorSize() const throw(std::exception);

        /** \brief Change the currently active acquisition to 
            *  \a newAcquisition  
            *
            *  This function will bring
            *  the system to Acquiring state and stop the
            *  previous acquisition if necessary.
            *
            *  \param newAcquisition Make this acquisition the active
            *                        one. */
        void setCurrentAcquisition(Acquisition *newAcquisition)
            throw(std::exception);
        /** \brief Ensures the system is Initialized or higher. */
        void ensureInitialized() throw() 
            { if (state < Initialized) goToState(Initialized); }

        /** \brief System-independent sleep() function.
            *
            *  \param milliseconds Number of ms to sleep
            **/
        static void sleep(const unsigned int milliseconds);

      private:
        /** Constructor. Called if necessary by getControl(). */
        Control() throw(std::exception);
        /** Undefined copy constructor. Do not call. */
        Control(const Control &c) throw(std::exception);
        /** Destructor. Will be called by the reference-counter. */
        virtual ~Control() throw(std::exception);

        /** Constructor helper that inits the config entries
            *  and connects them to the ConfigEntryHandler .
            */
        void initConfigEntries() throw(std::exception);

        friend class AndorCamera::ControlReference;
        /** Static pointer to the singleton instance of Control. */
        static Control* singletonInstance;
        /** Get a reference to the singleton Control object. If there
            *  is none yet, create it.
            *
            *  The Control object is reference counted. Thus, it is
            *  critical to release Control via the releaseControl()
            *  function. */
        static Control &getControl() throw(std::exception);
        /** Release the reference-counted Control object reference.
            *  If the count decreases to 0, deletes the Control object.*/
        static void releaseControl() throw(std::exception);

        /** This method is called if one of the triggers, the \c doCool
            *  element or the \c adChannel change. */
        void gotEvent(const ConfigEntry &) throw();

        /** The read mode to use for image acquisition. See 
            *  Andor SDK documentation for details. */
        static int readMode; 

        /** Current state of the selected camera. Do not manipulate
            *  directly; rather use the goToState method. */
        State state;
        /** The mutex for the \c state member */
        ost::Mutex stateMutex;

        /** Thread to monitor the temperature */
        std::auto_ptr<TemperatureMonitor> temperatureMonitor;

        /** Buffer for the camera detector size returned in 
            *  getDetectorSize(). */
        Dimension detectorSize;
        /** Is set to true if the camera has an internal shutter. */
        bool hasMechanicalShutter;

        /** If there is an active acquisition, this element points to
            *  it. Manipulated by the setCurrentAcquisition() function. */
        Acquisition *currentAcquisition;

        /** \var displayAtState
            *  The array element at index i holds a set of Config elements
            *  that should be viewable in state i.
            */
        /*  \var changeAtState
            *  The array element at index i holds a set of Config elements
            *  that should be editable in state i.
            *  */
        set<ConfigEntry*> displayAtState[6], changeAtState[6];
        /** \brief Display and unlock a config element only in
            *         the given state.
            *
            *  \param e Restrict the viewability of this element
            *  \param state Restrict the viewability to this state
            *  \sa viewable(), changeable()
            *  */
        void display(ConfigEntry& e, State state)
            { viewable(e, state); changeable(e, state); }
        /** \brief Display and unlock a config element only in the
            *         given states.
            *
            *  \param e    Restrict the viewability of this element
            *  \param from Lowest state to display this element in
            *  \param to   Highest state to display this element in
            *  \sa viewable(), changeable()
            *  */
        void display(ConfigEntry& e, State from, State to)
            { viewable(e, from, to); changeable(e, from, to); }
        /** \brief Display a config element only in the given state.
            *
            *  \param e     Restrict the viewability of this element
            *  \param state Restrict the viewability to this state
            *  */
        void viewable(ConfigEntry& e, State state)
            { if (state != Inquired) e.setViewable(false);
                displayAtState[state].insert(&e); }
        /** \brief Display a config element only in the given states.
            *
            *  \param e    Restrict the viewability of this element
            *  \param from Lowest state to display this element in
            *  \param to   Highest state to display this element in
            *  */
        void viewable(ConfigEntry& e, State from, State to) {
            if (from != Inquired) e.setViewable(false);
            for (int i = from; i <= to; i++)
                displayAtState[i].insert(&e); 
        }
        /** \brief Unlock a config element only in the given states.
            *
            *  \param e     Restrict the editability of this element
            *  \param state Restrict the viewability to this state
            *  */
        void changeable(ConfigEntry& e, State state)
            { if (state != Inquired) e.setEditable(false);
                changeAtState[state].insert(&e); }
        /** \brief Unlock a config element only in the given states.
            *
            *  \param e    Restrict the viewability of this element
            *  \param from Lowest state to display this element in
            *  \param to   Highest state to display this element in
            *  */
        void changeable(ConfigEntry& e, State from, State to) {
            if (from != Inquired) e.setEditable(false);
            for (int i = from; i <= to; i++)
                changeAtState[i].insert(&e); 
        }

        /** Master function to call for state transitions. 
            *  It will call the appropriate elementary state
            *  transition functions required to reach the new
            *  state.
            *  
            *  \param newState Arbitrary state to reach */
        void goToState(int newState) throw(std::exception);
        /** Increase state by one step. 
            *  Helper function for goToState() */
        void increaseState() throw(std::exception);
        /** Decrease state by one step. 
            *  Helper function for goToState() */
        void decreaseState() throw(std::exception);

        /** Unlock all elements that are in the new 
            *  \c changeAtState .
            *  Helper function for goToState() */
        void doEnable(int, int) throw();
        /** Lock all elements that are not in the new 
            *  \c changeAtState .
            *  Helper function for goToState() */
        void doDisable(int, int) throw();
        /** Show all elements that are in the new 
            *  \c displayAtState .
            *  Helper function for goToState() */
        void doShow(int, int) throw();
        /** Show all elements that are not in the new 
            *  \c displayAtState .
            *  Helper function for goToState() */
        void doHide(int, int) throw();

        /** \brief Elementery transition from Inquired to Initialized*/
        void initialize() throw(std::exception);
        /** \brief Elementery transition from Initialized to Inquired*/
        void finalize() throw(std::exception);

        /** \brief Elementery transition from Initialized to Cooling */
        void startCooling() throw(std::exception);
        /** \brief Elementery transition from Cooling to Initialized */
        void stopCooling() throw(std::exception);

        /** \brief Elementery transition from Cooling to Cooled */
        void ensureCooled() throw(std::exception);
        /** \brief Elementery transition from Cooled to Cooling */
        void allowWarming() throw(std::exception);

        /** \brief Elementery transition from Cooled to Configured */
        void configureCamera() throw(std::exception);
        /** \brief Elementery transition from Configured to Cooled */
        void disfigureCamera() throw(std::exception);

        /** \brief Elementery transition from Configured to Acquiring*/
        void startAcquiring() throw(std::exception);
        /** \brief Elementery transition from Acquiring to Configured*/
        void stopAcquiring() throw(std::exception);

        /** \brief Fill the \c adChannel choice element with the
            *         valid values. **/
        void fillADChannel() throw(std::exception);
        /** \brief Fill the \c VS_Speed choice element with the
            *         valid values. **/
        void fillVSSpeed() throw(std::exception);
        /** \brief Fill the \c HS_Speed choice element with the
            *         valid values.
            *
            *  Please note that these values depend on the setting
            *  of \c adChannel **/
        void fillHSSpeed() throw(std::exception);
    };
}
#endif
