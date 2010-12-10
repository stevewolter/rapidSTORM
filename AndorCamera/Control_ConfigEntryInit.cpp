/** \file 
 *  File containing the ConfigEntry initializations (names, connections,
 *  display states) for AndorCamera::Control 
 *  */
#include "AndorCamera/Control.h"
#include "SDK.h"
#include "Error.h"

namespace AndorCamera {

/* See the AndorCamera/Control.h header file for documentation */
void Control::initConfigEntries() throw(std::exception) {
    PROGRESS("Start");
    doCool.setUserLevel(ConfigEntry::Beginner);
    adChannel.setUserLevel(ConfigEntry::Intermediate);

    configDir.setName("CameraInitDir");
    configDir.setDesc("Location of camera's DETECTOR.INI");
    configDir.setHelp("Some Andor cameras must be initialized with "
                      "a DETECTOR.INI file and a .sys driver file "
                      "that define the camera's capabilities.");
    configDir.setUserLevel(ConfigEntry::Expert);
    /* Assume MS windows path structure */
    configDir = "..\\share\\andor";
    display(configDir, Inquired);

    chooseCamera.setName("ChooseCamera");
    chooseCamera.setDesc("Choose camera to connect to");
    long numCameras = GetAvailableCameras();
    PROGRESS("Found " << numCameras << " cameras");
    if (numCameras == 0) throw Error("No Andor camera found.");
    for (char i = 0; i < numCameras; i++)
        chooseCamera.addChoice
            ("Camera" + string(1, 'A'+i), "Camera " + string(1, 'A'+i));
    chooseCamera.setViewable( (numCameras > 1) );
    chooseCamera.setUserLevel(ConfigEntry::Beginner);
    display(chooseCamera, Inquired);
    
    connect.setName("ConnectToCamera");
    connect.setDesc("Connect to camera");
    connect.setUserLevel(ConfigEntry::Beginner);
    PROGRESS("First EH connect");
    ConfigEventHandler::connect(connect);
    PROGRESS("After First EH connect");
    display(connect, Inquired);

    disconnect.setName("DisconnectFromCamera");
    disconnect.setDesc("Shut down camera");
    disconnect.setUserLevel(ConfigEntry::Beginner);
    ConfigEventHandler::connect(disconnect);
    display(disconnect, Initialized, Acquiring);

    preCool.setName("PreCoolDivider");
    preCool.setUserLevel(ConfigEntry::Beginner);
    display(preCool, Initialized, Acquiring);

    targetTemperature.setMax(30);
    targetTemperature.setMin(-100);
    targetTemperature = -75;
    viewable(targetTemperature, Initialized, Acquiring);
    changeable(targetTemperature, Initialized);

    requiredTemperature.setName("TargetTemperature");
    requiredTemperature.setDesc("Required CCD temperature for "
                                "acquisition");
    requiredTemperature.setHelp("Cool the camera to this value before "
                                "starting acquisitions.");
    requiredTemperature.setUserLevel(ConfigEntry::Beginner);
    requiredTemperature.setMax(30);
    requiredTemperature.setMin(-100);
    requiredTemperature = -65;
    viewable(requiredTemperature, Initialized, Acquiring);
    changeable(requiredTemperature, Initialized, Cooling);

    viewable(doCool, Initialized, Acquiring);
    changeable(doCool, Initialized, Cooled);
    ConfigEventHandler::connect(doCool);

    realTemperature.setName("ActualTemperature");
    realTemperature.setDesc("Actual CCD temperature value");
    realTemperature.setHelp("This is the actual CCD temperature.");
    realTemperature.setUserLevel(ConfigEntry::Beginner);
    realTemperature.setEditable(false);
    /* realTemperature's visibilty is handled by the TemperatureMonitor
     * thread */
    realTemperature.setViewable(false);

    desired_VS_Speed.setName("DesiredVSSpeed");
    desired_VS_Speed.setDesc("Desired vertical shift time (µs)");
    desired_VS_Speed = 3.4;
    desired_VS_Speed.setUserLevel(ConfigEntry::Expert);
    display(desired_VS_Speed, Inquired);

    desired_HS_Speed.setName("DesiredHSSpeed");
    desired_HS_Speed.setDesc("Desired horizontal shift speed (MHz)");
    desired_HS_Speed = 10;
    desired_HS_Speed.setUserLevel(ConfigEntry::Expert);
    display(desired_HS_Speed, Inquired);

    adChannelDepth.setName("DesiredADChannelDepth");
    adChannelDepth.setDesc("Desired AD Channel Depth");
    adChannelDepth = 14;
    adChannelDepth.setUserLevel(ConfigEntry::Expert);
    display(adChannelDepth, Inquired);

    display(adChannel, Initialized, Cooled);
    /* We need connection to the adChannel to dynamically adjust the
     * HS_Speed values. */
    ConfigEventHandler::connect(adChannel);

    display(VS_Speed, Initialized, Cooled);
    display(HS_Speed, Initialized, Cooled);

    emccdGain = 200;
    display(emccdGain, Initialized, Cooled);

    exposureTime.setName("ExposureTime");
    exposureTime.setDesc("Desired exposure time (s)");
    exposureTime = 0.1;
    exposureTime.setUserLevel(ConfigEntry::Beginner);
    changeable(exposureTime, Inquired, Configured);

    realExposureTime.setUserLevel(ConfigEntry::Beginner);
    realExposureTime.setEditable(false);
    viewable(realExposureTime, Acquiring);
}

/* See the AndorCamera/Control.h header file for documentation */
void Control::registerEntries(ConfigSet &c) throw() {
    c.register_entry(&configDir);
    c.register_entry(&chooseCamera);
    c.register_entry(&connect);
    c.register_entry(&disconnect);

    c.register_entry(&preCool);
    c.register_entry(&targetTemperature);
    c.register_entry(&requiredTemperature);
    c.register_entry(&doCool);
    c.register_entry(&realTemperature);

    c.register_entry(&d1);
    c.register_entry(&desired_VS_Speed);
    c.register_entry(&desired_HS_Speed);
    c.register_entry(&adChannelDepth);
    c.register_entry(&VS_Speed);
    c.register_entry(&adChannel);
    c.register_entry(&HS_Speed);
    c.register_entry(&emccdGain);

    c.register_entry(&d2);
    c.register_entry(&exposureTime);
    c.register_entry(&realExposureTime);
}

}
