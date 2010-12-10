/** \file 
 *  The file for all functions of the Control class which
 *  did neither fit into Control_ConfigEntryInit.cpp nor
 *  Control_StateTransitions.cpp */
#define ANDORCAMERA_CONTROL_CPP
#include <cc++/thread.h>

#include <AndorCamera/Control.h>
#include <AndorCamera/ControlStateListener.h>
#include <AndorCamera/Acquisition.h>
#include <string.h>
#include <foreach.h>
#include "SDK.h"
#include "Error.h"
#include "TemperatureMonitor.h"

#include <math.h>

/** A macro to simplify error checking in direct calls to Andor
 *  SDK functions */
#define CHECK(x) { int line = __LINE__; checkAndorCode( x, line ); }

#define PRINT_STATE_TRANSITION_BEGIN(x,y) \
    STATUS("Going from state " << x << " to " << y)
#define PRINT_STATE_TRANSITION_END(x,y) \
    STATUS("Went from state " << x << " to " << y)

using namespace std;
using namespace ost;

namespace AndorCamera {

/** Mutex for \c refCounter and Control::singletonInstance */
static pthread_mutex_t refCounterMutex = PTHREAD_MUTEX_INITIALIZER;
/** Counts references on \c Control::singletonInstance */
static int refCounter = 0;
/** Pointer to the singleton instance of the Control class. Locked
 *  by \c refCounterMutex .
 *
 *  \sa ControlReference::ControlReference
 *  \sa Control::getControl 
 *  */
Control *Control::singletonInstance = NULL;

/* See AndorCamera/Control.h for documentation */
ControlReference::ControlReference() throw(std::exception)
: control(&Control::getControl())
{
}

/* See AndorCamera/Control.h for documentation */
ControlReference::ControlReference(const ControlReference&) throw() 
: control(&Control::getControl())
{}

/* See AndorCamera/Control.h for documentation */
ControlReference::~ControlReference() throw()
{
    try {
        if (control != NULL)
            Control::releaseControl();
    } catch (const std::exception &e) {
        cerr << PACKAGE_NAME << ": Error in AndorCamera control rollback: "
             << e.what() << endl;
    }
}

/* See AndorCamera/Control.h for documentation */
void ControlReference::release() throw(std::exception) {
    if (control != NULL) {
        control = NULL;
        Control::releaseControl();
    }
}

/* See AndorCamera/Control.h for documentation */
int Control::readMode = 4;

/* See AndorCamera/Control.h for documentation */
Control::Control() throw(std::exception) 
: currentAcquisition(NULL),
  doCool("Cooling", "Cool CCD", false),
  adChannel("ADChannel", "AD Channel")
{
    PROGRESS("Control Constructor");
    initConfigEntries();

    LOCKING("Config entries initialized");
    state = Inquired; 

    setName("AndorCamera");
    setDesc("Andor camera");
    registerEntries(*this);
}

/* See AndorCamera/Control.h for documentation */
Control::~Control() throw(std::exception) {
    ConfigEventHandler::disconnect(adChannel);
    ConfigEventHandler::disconnect(doCool);
    ConfigEventHandler::disconnect(connect);
    ConfigEventHandler::disconnect(disconnect);

    PROGRESS("Shutting down camera");
    goToState(Inquired);
    PROGRESS("Shutted camera down");
}

/* See AndorCamera/Control.h for documentation */
Control &Control::getControl() throw(std::exception) {
    PROGRESS("Acquiring control reference");
    MutexLock lock(refCounterMutex);

    if (singletonInstance == NULL) {
        PROGRESS("Allocating control");
        singletonInstance = new Control();
    }

    refCounter++;
    return *singletonInstance;
}

/* See AndorCamera/Control.h for documentation */
void Control::releaseControl() throw(std::exception) {
    MutexLock lock(refCounterMutex);
    refCounter--;
    if (refCounter == 0) {
        STATUS("Destructing singleton control");
        delete singletonInstance;
        singletonInstance = NULL;
        STATUS("Destructed singleton control");
    }
}

/* See AndorCamera/Control.h for documentation */
Control::Dimension Control::getDetectorSize() const throw(std::exception) {
    return detectorSize;
}

/* See AndorCamera/Control.h for documentation */
void Control::gotEvent(const ConfigEntry &e) throw() {
    MutexLock lock(stateMutex);
    try {
        /* Connect and disconnect are simple state changers. */
        if (&e == &connect) {
            goToState( Initialized );
            connect.untrigger();
        } else if (&e == &disconnect) {
            goToState( Inquired );
            disconnect.untrigger();
        } 
        /* The available horizontal scan speeds depend upon the A/D
         * channel. */
        else if (&e == &adChannel) {
            fillHSSpeed();
        } 
        /* The extra condition here prevents loops. The goToState
         * functions update the state and then update the doCool
         * variable; thus, if we are already updated, we may prevent
         * a loop if we break here. */
        else if (&e == &doCool) {
            if (doCool && state < Cooling) {
                goToState(Cooling);
            } else if ( !doCool && state >= Cooling ) {
                goToState(Initialized);
            }
        }
    } catch (const std::exception& exc) {
        cerr << PACKAGE_NAME << ": Control event for " << 
                e.name() << ": "<< exc.what() << endl;
    }
}

/* See AndorCamera/Control.h for documentation */
void Control::doShow(int from, int to) throw() {
    set<ConfigEntry*> &old(displayAtState[from]),
                        &fresh(displayAtState[to]);

    int maxEls = max<unsigned int>(old.size(), fresh.size());
    vector<ConfigEntry*> buf(maxEls);
    vector<ConfigEntry*>::iterator end, i;
    end = set_difference(fresh.begin(), fresh.end(), 
                    old.begin(), old.end(), buf.begin());
    for (i = buf.begin(); i != end; i++)
        (*i)->setViewable(true);
}

/* See AndorCamera/Control.h for documentation */
void Control::doHide(int from, int to) throw() {
    set<ConfigEntry*> &old(displayAtState[from]),
                        &fresh(displayAtState[to]);

    int maxEls = max<unsigned int>(old.size(), fresh.size());
    vector<ConfigEntry*> buf(maxEls);
    vector<ConfigEntry*>::iterator end, i;
    end = set_difference(old.begin(), old.end(), 
                    fresh.begin(), fresh.end(), buf.begin());
    for (i = buf.begin(); i != end; i++)
        (*i)->setViewable(false);
}

/* See AndorCamera/Control.h for documentation */
void Control::doEnable(int from, int to) throw() {
    set<ConfigEntry*> &old(changeAtState[from]),
                        &fresh(changeAtState[to]);

    int maxEls = max<unsigned int>(old.size(), fresh.size());
    vector<ConfigEntry*> buf(maxEls);
    vector<ConfigEntry*>::iterator end, i;
    end = set_difference(fresh.begin(), fresh.end(), 
                    old.begin(), old.end(), buf.begin());
    for (i = buf.begin(); i != end; i++)
        (*i)->setEditable(true);
}

/* See AndorCamera/Control.h for documentation */
void Control::doDisable(int from, int to) throw() {
    set<ConfigEntry*> &old(changeAtState[from]),
                        &fresh(changeAtState[to]);

    int maxEls = max<unsigned int>(old.size(), fresh.size());
    vector<ConfigEntry*> buf(maxEls);
    vector<ConfigEntry*>::iterator end, i;
    end = set_difference(old.begin(), old.end(), 
                    fresh.begin(), fresh.end(), buf.begin());
    for (i = buf.begin(); i != end; i++)
        (*i)->setEditable(false);
}

/* See AndorCamera/Control.h for documentation */
void Control::goToState(int newState) throw(std::exception) {
    /* This special case ensures that the showing and hiding of
     * display elements happens before and after the lengthy 
     * procedure of cooling. */
    if ( newState >= Cooled && state < Cooling ) {
        goToState(Cooling);
        goToState(newState);
    } else {
        /* The hiding and showing is done once at the start and
         * end of a complete transition to avoid "flickering" of
         * control elements. */
        int curState = state;
        PRINT_STATE_TRANSITION_BEGIN(curState, newState);
        doDisable(curState, newState);
        while (state != newState) {
            if (newState > state) increaseState(); else decreaseState();
        }
        doHide(curState, newState);
        doShow(curState, newState);
        doEnable(curState, newState);
        PRINT_STATE_TRANSITION_END(curState, newState);
    }
}

/* See AndorCamera/Control.h for documentation */
void Control::increaseState() throw(std::exception) {
    int curState = state;
    PRINT_STATE_TRANSITION_BEGIN(curState, "next higher");
    switch(curState) {
        case Inquired:      initialize();     break;
        case Initialized:   startCooling();   break;
        case Cooling:       ensureCooled();   break;
        case Cooled:        configureCamera();break;
        case Configured:    startAcquiring(); break;
        case Acquiring:    
        default:          throw Error("Can not raise state.");
    }
    PROGRESS("State after transition is " << state);
    ControlStateListener::notify(state);
    PRINT_STATE_TRANSITION_END(curState, state);
}

/* See AndorCamera/Control.h for documentation */
void Control::decreaseState() throw(std::exception) {
    int curState = state;
    PRINT_STATE_TRANSITION_BEGIN(curState, "next lower");
    switch(curState) {
        case Inquired:    throw Error("Can not lower state.");
        case Initialized: finalize();       break;
        case Cooling:     stopCooling();    break;
        case Cooled:      allowWarming();   break;
        case Configured:  disfigureCamera();break;
        case Acquiring:   stopAcquiring();  break;
        default:          throw Error("Can not lower state.");
    }
    ControlStateListener::notify(state);
    PRINT_STATE_TRANSITION_END(curState, state);
}

/* See AndorCamera/Control.h for documentation */
void Control::sleep(const unsigned int milliseconds) {
    Sleep(milliseconds);
}

/* See AndorCamera/Control.h for documentation */
void Control::fillADChannel() throw(std::exception) {
    int numADChannels, preferredADChannel = 0;
    CHECK( GetNumberADChannels(&numADChannels) );

    if (numADChannels != adChannel.numChoices())
        adChannel.removeAllChoices();

    for (int i = 0; i < numADChannels; i++) {
        int bitDepth = GetBitDepth( i );

        char buffer[20];
        sprintf(buffer, "%i bits", bitDepth);
        adChannel.addChoice(i, string(1, 'A'+i), string(buffer));
        
        if (bitDepth == (int)adChannelDepth())
            preferredADChannel = i;
    }
    adChannel = preferredADChannel;
    adChannel.setViewable( numADChannels > 1 );
}

/* See AndorCamera/Control.h for documentation */
void Control::fillVSSpeed() throw(std::exception) {
    int numVSSpeeds, preferredVSSpeed = -1;
    CHECK( GetNumberVSSpeeds(&numVSSpeeds) );

    if (numVSSpeeds != VS_Speed.numChoices())
        VS_Speed.removeAllChoices();

    float vals[numVSSpeeds];
    for (int i = 0; i < numVSSpeeds; i++) {
        vals[i] = GetVSSpeed(i);
        char buffer[20];
        sprintf(buffer, "%f µs", vals[i]);
        VS_Speed.addChoice(i, string(1, 'A'+i), string(buffer));
        
        if ( preferredVSSpeed == -1 ||
                  fabs(vals[preferredVSSpeed] - desired_VS_Speed())
                > fabs(vals[i] - desired_VS_Speed()) ) 
            preferredVSSpeed = i;
        //cerr << "Preferred VS speed of " << i << " is " << preferredVSSpeed << endl;
        //cerr << vals[i] << " " << vals[preferredVSSpeed] << endl;
    }
    VS_Speed = preferredVSSpeed;
    VS_Speed.setViewable( numVSSpeeds > 1 );
}

/* See AndorCamera/Control.h for documentation */
void Control::fillHSSpeed() throw(std::exception) {
    int adChannel = this->adChannel();
    int outputAmp = this->outputAmp();
    if (adChannel < 0 || adChannel >= this->adChannel.numChoices()) {
        HS_Speed.removeAllChoices();
    } else {
        int numHSSpeeds, preferredHSSpeed = -1;

        numHSSpeeds = GetNumberHSSpeeds(adChannel, (OutputAmp)outputAmp);
        if (numHSSpeeds != HS_Speed.numChoices())
            HS_Speed.removeAllChoices();

        float vals[numHSSpeeds];
        for (int i = 0; i < numHSSpeeds; i++) {
            vals[i] = GetHSSpeed(adChannel, (OutputAmp)outputAmp, i);
            char buffer[20];
            sprintf(buffer, "%f µs", vals[i]);
            HS_Speed.addChoice(i, string(1, 'A'+i), string(buffer));
            
            if ( preferredHSSpeed == -1 ||
                      fabs(vals[preferredHSSpeed] - desired_HS_Speed())
                    > fabs(vals[i] - desired_HS_Speed()) ) 
                preferredHSSpeed = i;
        }
        HS_Speed = preferredHSSpeed;
        HS_Speed.setViewable( numHSSpeeds > 1 );
    }
}

/* See AndorCamera/Control.h for documentation */
void Control::setCurrentAcquisition(Acquisition *a) throw(std::exception) {
    MutexLock lock(stateMutex);
    if ( a == currentAcquisition ) {
        /* Nothing */
    } else if ( a != NULL && currentAcquisition == NULL ) {
        goToState(Acquiring);
        currentAcquisition = a;
    } else if ( a != NULL && currentAcquisition != NULL ) {
        Acquisition *toDel = currentAcquisition;
        /* Remember to set \c currentAcquisition to NULL immediately
         * here since the call to Acquisition::stop will trigger another
         * invocation of this function. */
        currentAcquisition = NULL;
        toDel->stop();
        currentAcquisition = a;
    } else if ( a == NULL && currentAcquisition != NULL ) {
        currentAcquisition->stop();
        currentAcquisition = NULL;
        /* Going to the cooling state seems a safe default that allows
         * the user changes in the acquisition parameters. */
        goToState(Cooling);
    }
}

}
