#include "AndorCamera/Control.h"
#include "SDK.h"
#include "TemperatureMonitor.h"

/** \def NO_COOLER
 *  If the NO_COOLER macro is defined, the Control code will neither
 *  cool nor set the EMCCD gain level, but rather output a warning
 *  message about this macro. */
#ifndef NDEBUG
#define NO_COOLER
#endif

#define CHECK(x) { int line = __LINE__; checkAndorCode( x, line ); }

namespace AndorCamera {

/**  \details Connect to the camera selected with \c chooseCamera
  *  and initialize it. Determine temperature range and
  *  detector size.
  *
  *  Assumes the \c state to be inquired and sets it to
  *  initialized. */
void Control::initialize() throw(std::exception) {
    assert( state == Inquired );

    /* Select camera */
    long camHandle;
    CHECK( GetCameraHandle( chooseCamera(), &camHandle ) );
    CHECK( SetCurrentCamera( camHandle ) );

    /* Initialize camera */
    string dirstring = configDir();
    int len = dirstring.length()+1;
    char dir[len];
    strcpy(dir, dirstring.c_str());

    CHECK( Initialize( dir ) );

    /* Read temperature range */
    int minTemp, maxTemp;
    CHECK( GetTemperatureRange(&minTemp, &maxTemp) );
    targetTemperature.setMin(minTemp);
    targetTemperature.setMax(maxTemp);

    fillADChannel();
    fillVSSpeed();

    /* Read detector size */
    pair<int,int> s = GetDetector();
    detectorSize.w = s.first;
    detectorSize.h = s.second;
    hasMechanicalShutter = IsInternalMechanicalShutter();

    state = Initialized;
}

/**  \details Raises temperature to a safe level, stop monitoring the 
 *           temperature and deallocates all camera-related  
 *           variables including the temperature monitor thread.
  *
  *  Assumes the \c state to be \c Initialized and sets it to
  *  \c Inquired.
  **/
void Control::finalize() throw(std::exception) {
    assert( state == Initialized );

    if ( temperatureMonitor.get() == NULL )
        temperatureMonitor.reset( 
            new TemperatureMonitor( realTemperature ) );
    pair<bool, float> tstate;
    do {
        tstate = GetTemperatureF();
        realTemperature = tstate.second;
        Control::sleep(100);
    } while ( tstate.second < -20 );
    temperatureMonitor.reset(NULL);

    CHECK( ShutDown() );
    state = Inquired;
}

/** \details Start cooling the camera to \c targetTemperature and 
 *  starts the temperatureMonitor subthread to monitor the camera
 *  temperature. If NO_COOLER is set, does not actually cool.
 *
 *  Assumes the \c state to be \c Initialized and sets it to
 *  \c Cooling.
 **/
void Control::startCooling() throw(std::exception) {
    int coolTo = targetTemperature();
    CHECK(SetTemperature(coolTo));
#ifdef NO_COOLER
    cerr << "Would cool if I were allowed to" << endl;
#else
    CHECK(CoolerON());
#endif

    state = Cooling;
    if (doCool == false) doCool.setValue(true);
    /* We need to check for the existence of the \c temperatureMonitor
     * here because finalize() stops that process, not stopCooling().
     * Thus, it might still be active if we went through the states
     * Inquired -> Initialized -> Cooling -> Initialized . */
    if ( temperatureMonitor.get() == NULL )
        temperatureMonitor.reset( 
            new TemperatureMonitor( realTemperature ) );
}

/** \details Stops cooling the camera and updates the doCool variable.
 *  
 *  Assumes the \c state to be \c Cooling and sets it to
 *  \c Initialized.
 **/
void Control::stopCooling() throw(std::exception) {
    assert( state == Cooling );

#ifndef NO_COOLER
    CHECK(CoolerOFF());
#else
    cerr << "Would stop cooler if I were allowed to" << endl;
#endif

    state = Initialized;
    if (doCool == true) doCool.setValue(false);
}

/** \details This function blocks control flow until the camera is cooled
 *          to \c requiredTemperature. This might take a long
  *         time.
  *  
  *  Assumes the \c state to be \c Cooling and sets it to
  *  \c Cooled.
  **/
void Control::ensureCooled() throw(std::exception) {
    assert( state == Cooling );

#ifndef NO_COOLER
    int coolTo = requiredTemperature();
    pair<bool, float> tstate;
    do {
        tstate = GetTemperatureF();
        sleep(1000);
    } while ( tstate.first == false && tstate.second > coolTo );
#endif

    state = Cooled;
}

/** \details Leaves the cooled state and allows to switch the
 *         cooler off.
 *  
 *  Assumes the \c state to be \c Cooled and sets it to
 *  \c Cooling.
 **/
void Control::allowWarming() throw(std::exception) {
    assert( state == Cooled );

    state = Cooling;
}

/** \details Freezes the readout speeds, modes and various
 *          acquisition parameters and transmits them to
 *          the camera.
 *  
 *  Assumes the \c state to be \c Cooled and sets it to
 *  \c Configured.
 **/
void Control::configureCamera() throw(std::exception) {
    assert( state == Cooled );

    if (adChannel.isValid())
        SetADChannel( adChannel() );
    if (HS_Speed.isValid()) {
        SetHSSpeed( outputAmp(), HS_Speed() );
    }
    if (VS_Speed.isValid())
        SetVSSpeed( VS_Speed() );
#ifndef NO_COOLER
    /* If the cooler is off, EMCCD gain might damage the camera.
     * Leave it off. */
    CHECK( SetEMCCDGain( emccdGain() ) );
#endif
    if (hasMechanicalShutter) {
        SetShutter( 1/* TTL High */, 1 /* Open */, 10, 10 );
        CHECK( SetTriggerMode( 0 /* Internal */ ) );
    }

    CHECK( SetReadMode( readMode ) );
    CHECK( SetFrameTransferMode( 1 /* ON */ ) );

    state = Configured;
}

/** \details Thaws the acquisition parameters. Changes to the
 *          control elements are allowed again.
 *  
 *  Assumes the \c state to be \c Configured and sets it to
 *  \c Cooled.
 **/
void Control::disfigureCamera() throw(std::exception) {
    assert( state == Configured );

    if (hasMechanicalShutter)
        SetShutter( 1/* TTL High */, 2 /* Closed */, 10, 10 );

    state = Cooled;
}

/** \details Starts an acquisition on the camera. The temperature
 *  monitor needs to be stopped (for GetTemperatureF returns
 *  DRV_ACQUIRING during acquisitions).
 *  
 *  Assumes the \c state to be \c Configured and sets it to
 *  \c Acquiring.
 **/
void Control::startAcquiring() throw(std::exception) {
    int armor[100];
    armor[0] = 0;
    assert( state == Configured );

    temperatureMonitor.reset(NULL);
    PROGRESS("Destructed temperature monitor");

    SetKineticCycleTime( exposureTime() );
    SetExposureTime( exposureTime() );

    PROGRESS("Checking exposure time");
    realExposureTime = GetExposureTime();
    PROGRESS("Finished startAcquiring");

    state = Acquiring;
}

/** \details Stops the running acquisition on the camera and restarts
 *  temperature monitoring.
 *  
 *  Assumes the \c state to be \c Acquiring and sets it to
 *  \c Configured.
 **/
void Control::stopAcquiring() throw(std::exception) {
    assert( state == Acquiring );
    state = Configured;
    temperatureMonitor.reset( 
        new TemperatureMonitor( realTemperature ) );
}

}
