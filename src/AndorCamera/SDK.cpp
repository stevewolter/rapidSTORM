#define ANDORCAMERA_SDK_CPP

#include "SDK.h"
#include <iostream>
#include <dStorm/helpers/thread.h>
#include <cassert>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef LINK_SDK_DYNAMICALLY
/* This sounds ridiculous, but, for some reason, these includes
 * are necessary for linking the ATMCD32D.H with mingw. */
#include <stdexcept>
#include <windows.h>
#include <ATMCD32D_dynamic.h>

class DynamicLibrary {
  public:
    typedef HMODULE Handle;
  private:
    Handle handle;
  public:
    DynamicLibrary (const char *file) {
        handle = LoadLibrary(file);
        if ( handle == NULL )
            throw std::runtime_error("Dynamic library " + 
                std::string(file) + " not found.");
    }
    ~DynamicLibrary() { FreeLibrary(handle); }
    Handle get() { return handle; }
};

static DynamicLibrary& andor_lib() {
    static DynamicLibrary lib("ATMCD32D.DLL");
    return lib;
}

FARPROC SafeGetProcAddress( DynamicLibrary& library, 
                            const char * name )
{
    FARPROC ptr = GetProcAddress( library.get(), name );
    if (ptr == NULL) {
        throw std::logic_error("Unable to find function " + std::string(name) + " in Andor SDK library." );
    }
    return ptr;
}

template <typename Function>
class DynamicBind {
  public:
    static Function call(const char *name) {
        return (Function)SafeGetProcAddress( andor_lib(), name );
    }
};

#define USE_FUNCTION(x) \
    static x ## Ptr x = DynamicBind<x ## Ptr>::call(#x);

USE_FUNCTION(AbortAcquisition)
USE_FUNCTION(CoolerOFF)
USE_FUNCTION(CoolerON)
USE_FUNCTION(FreeInternalMemory)
USE_FUNCTION(GetAcquisitionTimings)
USE_FUNCTION(GetAvailableCameras)
USE_FUNCTION(GetBitDepth)
USE_FUNCTION(GetCameraHandle)
USE_FUNCTION(GetDetector)
USE_FUNCTION(GetEMGainRange)
USE_FUNCTION(GetHSSpeed)
USE_FUNCTION(GetImages16)
USE_FUNCTION(GetNumberADChannels)
USE_FUNCTION(GetNumberAvailableImages)
USE_FUNCTION(GetNumberHSSpeeds)
USE_FUNCTION(GetNumberNewImages)
USE_FUNCTION(GetNumberVSSpeeds)
USE_FUNCTION(GetStatus)
USE_FUNCTION(GetTemperatureF)
USE_FUNCTION(GetTemperatureRange)
USE_FUNCTION(GetVSSpeed)
USE_FUNCTION(Initialize)
USE_FUNCTION(IsInternalMechanicalShutter)
USE_FUNCTION(SetAcquisitionMode)
USE_FUNCTION(SetADChannel)
USE_FUNCTION(SetCurrentCamera)
USE_FUNCTION(SetEMCCDGain)
USE_FUNCTION(SetExposureTime)
USE_FUNCTION(SetFrameTransferMode)
USE_FUNCTION(SetHSSpeed)
USE_FUNCTION(SetImage)
USE_FUNCTION(SetKineticCycleTime)
USE_FUNCTION(SetNumberKinetics)
USE_FUNCTION(SetReadMode)
USE_FUNCTION(SetShutter)
USE_FUNCTION(SetTemperature)
USE_FUNCTION(SetTriggerMode)
USE_FUNCTION(SetVSSpeed)
USE_FUNCTION(ShutDown)
USE_FUNCTION(StartAcquisition)
USE_FUNCTION(WaitForAcquisition)

#else /* HAVE_WINDOWS_H */
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#if defined(HAVE_LIBATMCD32D)
#include <ATMCD32D.H>
#elif defined(HAVE_LIBDUMMYANDORCAMERA)
#include <ATMCD32D_for_dummy.h>
#endif
#endif

#define CHECK(x,y) try { checkAndorCodeStr( x, y ); } catch (Error& e) { throw e; }

#if DEBUG_LEVEL >= 10
//#undef STATUS
//#define STATUS(x) cerr << x << endl;
#define ENTRY(x) cerr << "Entry in " << x << endl;
#define EXIT(x) cerr << "Exit from " << x << endl;
#else
#define ENTRY(x)
#define EXIT(x)
#endif

using namespace std;

static const int armorInt = 0x95959595;
static const float armorFloat = 0;

namespace SDK {

static void checkAndorCodeStr( unsigned int return_code, 
                               const char *place )

{
    if (return_code != DRV_SUCCESS) {
        STATUS("Got Andor error " << return_code);
        throw Error(return_code, place);
    }
}

void checkAndorCode( int rc, int line ) {
    char buf[20];
    sprintf(buf, " at line %i", line);
    checkAndorCodeStr(rc, (const char *)buf);
}

static void clearStack(const int [], unsigned char n = 0) {
    int clearArray[100];
    memset( clearArray, n, 100*sizeof(int) );
}

static void clearStack(const float []) {
    int clearArray[100];
    memset( clearArray, 0, 100*sizeof(int) );
}

static void initArmor(int a[], int sz) {
    for (int i = 0; i < sz; i++)
        a[i] = armorInt;
}

static void checkArmor(int a[], int sz, const char *where)
 {
    int i = 0;
    for (i = 0; i < sz; i++) {
        if (a[i] != armorInt)
            cerr << "Armor penetrated by " << where << " with "
                 << i << " bytes left." << endl;
    }
}

#if 0
static void printStatus() {
    int armor[40];
    CHECK(::GetStatus(&armor[10]), " in GetStatus");
    cerr << "Camera status is " << armor[10] << endl;
}
#endif

long GetAvailableCameras() {
    ENTRY("GetAvailableCameras");
    long n;
    CHECK( ::GetAvailableCameras( &n ) , " in GetAvailableCameras" );
    EXIT("GetAvailableCameras");
    return n;
}

pair<bool,float> GetTemperatureF() {
    ENTRY("GetTemperatureF");
    int armor[20];
    pair<bool,float> result;

    clearStack(armor);
    int rc = ::GetTemperatureF( &result.second );

    switch(rc) {
        case DRV_TEMPERATURE_NOT_REACHED:
        case DRV_TEMPERATURE_DRIFT:
        case DRV_TEMPERATURE_OFF:
        case DRV_TEMPERATURE_NOT_STABILIZED:
            result.first = false;
            break;
        case DRV_TEMPERATURE_STABILIZED:
            result.first = true;
            break;
        default:
            CHECK( rc, " in GetTemperatureF" );
    };
    EXIT("GetTemperatureF");
    return result;
}

void SetADChannel( ADChannel index ) 
{
    ENTRY("SetADChannel");
    int armor[20];
    clearStack(armor);
    
    CHECK( ::SetADChannel( index ), " in SetADChannel" );
    EXIT("SetADChannel");
}

int GetBitDepth( ADChannel index ) {
    ENTRY("GetBitDepth");
    int armorbuf[20];
    clearStack(armorbuf);

    CHECK( ::GetBitDepth( index, &armorbuf[10] ), " in GetBitDepth" );

    EXIT("GetBitDepth");
    return armorbuf[10];
}

int GetNumberHSSpeeds( ADChannel adChan, OutputAmp o ) 

{
    ENTRY("GetNumberHSSpeeds");
    int armorbuf[20];

    clearStack(armorbuf);
    CHECK( ::GetNumberHSSpeeds( adChan, o, &(armorbuf[10]) ),
            " in GetNumberHSSpeeds" );

    EXIT("GetNumberHSSpeeds");
    return armorbuf[10];
}

float GetHSSpeed( ADChannel adChan, OutputAmp o, int index )

{
    ENTRY("GetHSSpeed");
    float armoredspeed[20];
    clearStack(armoredspeed);
    CHECK( ::GetHSSpeed( adChan, o, index, &(armoredspeed[11])),
                  " in GetHSSpeed");
    clearStack(armoredspeed);
    EXIT("GetHSSpeed");
    return armoredspeed[11];
}

float GetVSSpeed( int index ) {
    ENTRY("GetVSSpeed");
    int armor[20];
    float result;

    clearStack(armor);
    CHECK( ::GetVSSpeed( index, &result ), " in GetVSpeed" );
    EXIT("GetVSSpeed");
    return result;
}

void SetVSSpeed( int index )
{
    ENTRY("SetVSSpeed");
    int armor[20];
    clearStack(armor);
    CHECK( ::SetVSSpeed( index ), " in SetVSpeed" );
    EXIT("SetVSSpeed");
}
void SetHSSpeed( int amplification, int index )

{
    ENTRY("SetHSSpeed");
    int armor[20];
    clearStack(armor);
    CHECK( ::SetHSSpeed( amplification, index ), " in SetHSpeed" );
    EXIT("SetHSSpeed");
}

std::pair<int,int> GetDetector() {
    ENTRY("GetDetector");
    int armor[40];
    clearStack(armor);

    CHECK( ::GetDetector( &armor[2], &armor[3] ), " in GetDetector" );
    EXIT("GetDetector");
    return std::pair<int,int>(armor[2], armor[3]);
}

void realSetShutter(int highety, int mode, int op_time, int cl_time)
 {
    CHECK( ::SetShutter(highety, mode, op_time, cl_time), 
           " in SetShutter" );
}

bool IsInternalMechanicalShutter() {
    ENTRY("IsInternalMechanicalShutter");
    int armor[40];
    clearStack(armor);

    CHECK( ::IsInternalMechanicalShutter(&armor[20]), " in IIMS" );

    EXIT("IsInternalMechanicalShutter");
    return &armor[20];
}

void SetShutter( Shutter::TTL highety, Shutter::Mode mode, int op_time,
    int cl_time )
{
    ENTRY("SetShutter");
    int armor[40];
    clearStack(armor);
    initArmor(armor, 40);

    realSetShutter(highety, mode, op_time, cl_time);

    checkArmor(armor, 40, "SetShutter");
    EXIT("SetShutter");
}

void SetImage( int binx, int biny, 
                int firstCol, int lastCol, int firstRow, int lastRow )

{
    int armor[120];
    clearStack(armor, 0xFF);
    ENTRY("SetImage with " << binx << " " << biny << " " << firstCol <<
          " " << lastCol << " " << firstRow << " " << lastRow);

    CHECK( ::SetImage( binx, biny, firstCol+1, lastCol+1, firstRow+1, lastRow+1 ),
        " in SetImage" );
    EXIT("SetImage");
}

void SetImageNoBinning(int firstCol, int lastCol, 
                        int firstRow, int lastRow )
{ 
    SetImage(1, 1, firstCol, lastCol, firstRow, lastRow); 
}


NewImages GetNumberAvailableImages()
 
{
    ENTRY("GetNumberAvailableImages");
    at_32 f, l;
    NewImages rv;

    rv.haveNew = false;

    int armor[50];
    armor[49] = 5;

    int rc = ::GetNumberAvailableImages(&f, &l);
    rv.first = f;
    rv.second = l;
    if (rc == DRV_NO_NEW_DATA)
        rv.haveNew = false;
    else if (rc == DRV_SUCCESS)
        rv.haveNew = true;
    else
        CHECK(rc, " in GetNumberAvailableImages");

    rv.first -= 1;
    rv.second -= 1;

    EXIT("GetNumberAvailableImages");
    return rv;
}

NewImages GetNumberNewImages()
 
{
    ENTRY("GetNumberNewImages");
    NewImages rv;

    rv.haveNew = false;

    long int armor[50];
    armor[49] = 5;

    int rc = ::GetNumberNewImages(armor+33, armor+35);
    if (rc == DRV_NO_NEW_DATA)
        rv.haveNew = false;
    else if (rc == DRV_SUCCESS)
        rv.haveNew = true;
    else
        CHECK(rc, " in GetNumberNewImages");

    rv.first = armor[33] - 1;
    rv.second = armor[35] - 1;

    EXIT("GetNumberNewImages");
    return rv;
}

void SetAcquisitionMode(AndorCamera::AcquisitionMode mode) {
    ENTRY("SetAcquisitionMode");
    int armor[200];
    initArmor(armor, 200);
    clearStack(armor);
    CHECK( ::SetAcquisitionMode(int(mode)), " in SetAcquisitionMode" );
    EXIT("SetAcquisitionMode");
}

void SetNumberKinetics( int number ) {
    ENTRY("SetNumberKinetics");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetNumberKinetics(number), " in SetNumberKinetics" );
    EXIT("SetNumberKinetics");
}

void SetKineticCycleTime( float number ) {
    ENTRY("SetKineticCycleTime");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetKineticCycleTime(number), " in SetKineticCycleTime" );
    EXIT("SetKineticCycleTime");
}

void SetExposureTime( float number ) {
    ENTRY("SetExposureTime");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetExposureTime(number), " in SetExposureTime" );
    EXIT("SetExposureTime");
}

float GetExposureTime() {
    ENTRY("GetExposureTime");
    float v[3];
    CHECK( ::GetAcquisitionTimings( &v[0], &v[1], &v[2] ),
           " in GetExposureTime" );
    EXIT("GetExposureTime");
    return v[0];
}

float GetAccumulationCycleTime() {
    ENTRY("GetAccumulationCycleTime");
    float armor[40];
    clearStack(armor);
    CHECK( ::GetAcquisitionTimings( &armor[19], &armor[20], &armor[21] ),
           " in GetAccumulationCycleTime" );
    EXIT("GetAccumulationCycleTime");
    return armor[20];
}

float GetKineticCycleTime() {
    ENTRY("GetKineticCycleTime");
    float armor[40];
    clearStack(armor);
    CHECK( ::GetAcquisitionTimings( &armor[19], &armor[20], &armor[21] ),
           " in GetKineticCycleTime" );
    EXIT("GetKineticCycleTime");
    return armor[21];
}

void StartAcquisition() {
    ENTRY("StartAcquisition");
    int armor[10];
    armor[2] = 0;
    CHECK( ::StartAcquisition(), " in StartAcquisition" );
    EXIT("StartAcquisition");
}

AcquisitionState GetImages16
    (Range range, uint16_t *buf,
                unsigned long bufSize, Range &read)
{
    assert( sizeof(WORD) == sizeof(uint16_t) );

    long int signed_read_first = read.first,
             signed_read_second = read.second;
    ENTRY("GetImages16");
    int rv = ::GetImages16(
                    range.first+1, range.second+1,
                    buf, bufSize,
                    &signed_read_first, &signed_read_second );
    read.first = signed_read_first - 1;
    read.second = signed_read_second - 1;

    EXIT("GetImages16");
    if ( rv == DRV_GENERAL_ERRORS )
        return Missed_Images;
    else if ( rv == DRV_NO_NEW_DATA )
        return No_New_Images;
    else
        return New_Images;
}

void Initialize(std::string init_file_dir) 
{
    ENTRY("Initialize");
    /* Initialize camera */
    int len = init_file_dir.length()+1;
    char dir[len];
    strcpy(dir, init_file_dir.c_str());

    CHECK( ::Initialize( dir ), " in Initialize" );
    EXIT("Initialize");
}

AcquisitionState WaitForAcquisition() {
    ENTRY("WaitForAcquisition");
    int rv = ::WaitForAcquisition();
    EXIT("WaitForAcquisition");
    if ( rv == DRV_SUCCESS )
        return New_Images;
    else if ( rv == DRV_NO_NEW_DATA )
        return No_New_Images;
    else        
        throw Error( rv, " in WaitForAcquisition" );
}

CameraState GetStatus() 
{
    int armor[40];
    armor[23] = -1;

    ENTRY("GetStatus");
    CHECK( ::GetStatus(armor+23), " in GetStatus" );
    EXIT("GetStatus");
    if ( armor[23] == DRV_IDLE )
        return Idle;
    else if ( armor[23] == DRV_TEMPCYCLE )
        return Temperature_Cycle;
    else if ( armor[23] == DRV_ACQUIRING )
        return Is_Acquiring;
    else if ( armor[23] == DRV_ACCUM_TIME_NOT_MET )
        return Accum_time_not_met;
    else if ( armor[23] == DRV_KINETIC_TIME_NOT_MET )
        return Kinetic_time_not_met;
    else if ( armor[23] == DRV_ERROR_ACK )
        return Error_ACK;
    else if ( armor[23] == DRV_ACQ_BUFFER )
        return Acq_Buffer;
    else if ( armor[23] == DRV_SPOOLERROR )
        return Spool_Error;
    else
        throw Error(0, "Invalid status code");

}

void FreeInternalMemory() {
    ENTRY("FreeInternalMemory");
    CHECK( ::FreeInternalMemory(), " in FreeInternalMemory" );
    EXIT("FreeInternalMemory");
}

bool AbortAcquisition() {
    ENTRY("AbortAcquisition");
    long int armor[40];
    armor[20] = ::AbortAcquisition();
    EXIT("AbortAcquisition");
    if ( armor[20] == DRV_SUCCESS )
        return true;
    else if ( armor[20] == DRV_IDLE )
        return false;
    else {
        CHECK( armor[20], " in AbortAcquisition" );
        return true;
    }
}

void SetTemperature(int temperature) {
    ENTRY("SetTemperature");
    CHECK( ::SetTemperature(temperature), " in SetTemperature" );
    EXIT("SetTemperature");
}

void CoolerON() {
    ENTRY("CoolerON");
    CHECK( ::CoolerON(), " in CoolerON" );
    EXIT("CoolerON");
}
void CoolerOFF() {
    ENTRY("CoolerOFF");
    CHECK( ::CoolerOFF(), " in CoolerOFF" );
    EXIT("CoolerOFF");
}

pair<int,int> GetTemperatureRange() {
    ENTRY("GetTemperatureRange");
    int armor[50];

    CHECK( ::GetTemperatureRange(armor+20, armor+22), 
           " in GetTemperatureRange" );

    EXIT("GetTemperatureRange: " << armor[20] << " " << armor[22]);
    return pair<int,int>(armor[20], armor[22]);
}

CameraHandle GetCameraHandle(int index) {
    long armor[50];
    
    ENTRY("GetCameraHandle");
    CHECK( ::GetCameraHandle(index, armor+23), " in GetCameraHandle " );
    EXIT("GetCameraHandle");

    return armor[23];
}
void SetCurrentCamera(CameraHandle handle) {
    int armor[40];
    ENTRY("SetCurrentCamera");
    armor[23] = 42;
    CHECK( ::SetCurrentCamera(handle), " in SetCurrentCamera" );
    EXIT("SetCurrentCamera");
}
void ShutDown() {
    int armor[40];
    armor[23] = 42;
    CHECK( ::ShutDown(), " in ShutDown" );
}

void SetTriggerMode( Trigger::Mode mode ) {
    ENTRY("SetTriggerMode");
    int armor[40];
    armor[5] = 23;
    CHECK( ::SetTriggerMode(int(mode)), " in SetTriggerMode" );
    EXIT("SetTriggerMode");
}

void SetFrameTransferMode( bool active ) {
    ENTRY("SetFrameTransferMode");
    int armor[40];
    armor[5] = 23;
    CHECK( ::SetFrameTransferMode( (active) ? 1 : 0), " in SetFrameTransferMode" );
    EXIT("SetFrameTransferMode");
}

void SetEMCCDGain( int gain ) {
    ENTRY("SetEMCCDGain");
    int armor[40];
    armor[5] = 23;
    CHECK( ::SetEMCCDGain(gain), " in SetEMCCDGain" );
    EXIT("SetEMCCDGain");
}

void SetReadMode( AndorCamera::ReadoutMode mode ) {
    ENTRY("SetReadMode");
    int armor[40];
    armor[5] = 23;
    CHECK( ::SetReadMode(mode), " in SetReadMode" );
    EXIT("SetReadMode");
}

int GetNumberADChannels() {
    ENTRY("GetNumberADChannels");
    int armor[40];
    armor[5] = 42;
    CHECK( ::GetNumberADChannels(armor+23), " in GetNumberADChannels" );
    EXIT("GetNumberADChannels");
    return armor[23];
}

int GetNumberVSSpeeds() {
    ENTRY("GetNumberVSSpeeds");
    int armor[40];
    armor[5] = 42;
    CHECK( ::GetNumberVSSpeeds(armor+23), " in GetNumberVSSpeeds" );
    EXIT("GetNumberVSSpeeds");
    return armor[23];
}

std::pair<int,int> GetEMCCDGainRange() {
    std::pair<int,int> rv;
    CHECK( ::GetEMGainRange( &rv.first, &rv.second ), " in GetEMCCDGainRange" );
    return rv;
}

}
