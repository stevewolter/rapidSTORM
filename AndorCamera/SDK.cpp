#define ANDORCAMERA_SDK_CPP

#include "SDK.h"
#include <iostream>
#include <cc++/thread.h>

#define CHECK(x,y) try { checkAndorCodeStr( x, y ); } catch (Error& e) { throw e; }

#if DEBUG_LEVEL >= 10
#define ENTRY(x) cerr << "Entry in " << x << endl;
#define EXIT(x) cerr << "Exit from " << x << endl;
#else
#define ENTRY(x)
#define EXIT(x)
#endif

using namespace AndorCamera;
using namespace std;

static const int armorInt = 0x95959595;
static const float armorFloat = 0;

static void checkAndorCodeStr( unsigned int return_code, 
                               const char *place )
throw(Error)
{
    STATUS("Got Andor error " << return_code);
    if (return_code != DRV_SUCCESS) {
        throw Error(return_code, place);
    }
}

void AndorCamera::checkAndorCode( int rc, int line ) throw(Error) {
    char buf[20];
    sprintf(buf, " at line %i", line);
    checkAndorCodeStr(rc, (const char *)buf);
}

static void clearStack(const int []) throw() {
    int clearArray[100];
    memset( clearArray, 0, 100*sizeof(int) );
}

static void clearStack(const float []) throw() {
    int clearArray[100];
    memset( clearArray, 0, 100*sizeof(int) );
}

static void initArmor(int a[], int sz) throw() {
    for (int i = 0; i < sz; i++)
        a[i] = armorInt;
}

static void initArmor(float a[], int sz) throw() {
    for (int i = 0; i < sz; i++)
        a[i] = armorFloat;
}

static void checkArmor(int a[], int sz, const char *where)
throw() {
    int i = 0;
    for (i = 0; i < sz; i++) {
        if (a[i] != armorInt)
            cerr << "Armor penetrated by " << where << " with "
                 << i << " bytes left." << endl;
    }
}

static void printStatus() throw() {
    int armor[40];
    CHECK(::GetStatus(&armor[10]), " in GetStatus");
    cerr << "Camera status is " << armor[10] << endl;
}

long AndorCamera::GetAvailableCameras() throw(Error) {
    long n;
    CHECK( ::GetAvailableCameras( &n ) , " in GetAvailableCameras" );
    return n;
}

pair<bool,float> AndorCamera::GetTemperatureF() throw(Error) {
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

void AndorCamera::SetADChannel( ADChannel index ) throw(Error) 
{
    ENTRY("SetADChannel");
    int armor[20];
    clearStack(armor);
    
    CHECK( ::SetADChannel( index ), " in SetADChannel" );
    EXIT("SetADChannel");
}

int AndorCamera::GetBitDepth( ADChannel index ) throw(Error) {
    ENTRY("GetBitDepth");
    int armorbuf[20];
    clearStack(armorbuf);

    CHECK( ::GetBitDepth( index, &armorbuf[10] ), " in GetBitDepth" );

    EXIT("GetBitDepth");
    return armorbuf[10];
}

int AndorCamera::GetNumberHSSpeeds( ADChannel adChan, OutputAmp o ) 
    throw(Error)
{
    ENTRY("GetNumberHSSpeeds");
    int armorbuf[20];

    clearStack(armorbuf);
    CHECK( ::GetNumberHSSpeeds( adChan, o, &(armorbuf[10]) ),
            " in GetNumberHSSpeeds" );

    EXIT("GetNumberHSSpeeds");
    return armorbuf[10];
}

float AndorCamera::GetHSSpeed( ADChannel adChan, OutputAmp o, int index )
    throw(Error)
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

float AndorCamera::GetVSSpeed( int index ) throw(Error) {
    ENTRY("GetVSSpeed");
    int armor[20];
    float result;

    clearStack(armor);
    CHECK( ::GetVSSpeed( index, &result ), " in GetVSpeed" );
    EXIT("GetVSSpeed");
    return result;
}

void AndorCamera::SetVSSpeed( int index ) throw(Error)
{
    ENTRY("SetVSSpeed");
    int armor[20];
    clearStack(armor);
    CHECK( ::SetVSSpeed( index ), " in SetVSpeed" );
    EXIT("SetVSSpeed");
}
void AndorCamera::SetHSSpeed( int amplification, int index )
    throw(Error)
{
    ENTRY("SetHSSpeed");
    int armor[20];
    clearStack(armor);
    CHECK( ::SetHSSpeed( amplification, index ), " in SetHSpeed" );
    EXIT("SetHSSpeed");
}

std::pair<int,int> AndorCamera::GetDetector() throw(Error) {
    ENTRY("GetDetector");
    int armor[40];
    clearStack(armor);

    CHECK( ::GetDetector( &armor[2], &armor[3] ), " in GetDetector" );
    EXIT("GetDetector");
    return std::pair<int,int>(armor[2], armor[3]);
}

void realSetShutter(int highety, int mode, int op_time, int cl_time)
throw(Error) {
    CHECK( ::SetShutter(highety, mode, op_time, cl_time), 
           " in SetShutter" );
}

bool AndorCamera::IsInternalMechanicalShutter() throw(Error) {
    ENTRY("IsInternalMechanicalShutter");
    int armor[40];
    clearStack(armor);

    CHECK( ::IsInternalMechanicalShutter(&armor[20]), " in IIMS" );

    EXIT("IsInternalMechanicalShutter");
    return &armor[20];
}

void AndorCamera::SetShutter( int highety, int mode, int op_time,
    int cl_time ) throw(Error)
{
    ENTRY("SetShutter");
    int armor[40];
    clearStack(armor);
    initArmor(armor, 40);

    realSetShutter(highety, mode, op_time, cl_time);

    checkArmor(armor, 40, "SetShutter");
    EXIT("SetShutter");
}

void AndorCamera::SetImage( int binx, int biny, 
                int firstCol, int lastCol, int firstRow, int lastRow )
    throw(Error)
{
    ENTRY("SetImage");
    int armor[30];
    clearStack(armor);

    CHECK( ::SetImage( binx, biny, firstCol+1, lastCol+1, firstRow+1, lastRow+1 ),
        " in SetImage" );
    EXIT("SetImage");
}

void AndorCamera::SetImageNoBinning(int firstCol, int lastCol, 
                        int firstRow, int lastRow ) throw(Error)
{ 
    SetImage(1, 1, firstCol, lastCol, firstRow, lastRow); 
}


AndorCamera::NewImages AndorCamera::GetNumberAvailableImages()
throw(Error) 
{
    ENTRY("GetNumberAvailableImages");
    NewImages rv;

    rv.haveNew = false;

    int armor[50];
    armor[49] = 5;

    int rc = ::GetNumberAvailableImages(&rv.first, &rv.second);
    if (rc == DRV_NO_NEW_DATA)
        rv.haveNew = false;
    else if (rc == DRV_SUCCESS)
        rv.haveNew = true;
    else
        CHECK(rc, " in GetNumberNewImages");

    rv.first -= 1;
    rv.second -= 1;

    EXIT("GetNumberAvailableImages");
    return rv;
}

AndorCamera::NewImages AndorCamera::GetNumberNewImages()
throw(Error) 
{
    ENTRY("GetNumberNewImages");
    NewImages rv;

    rv.haveNew = false;

    int armor[50];
    armor[49] = 5;

    int rc = ::GetNumberNewImages(&rv.first, &rv.second);
    if (rc == DRV_NO_NEW_DATA)
        rv.haveNew = false;
    else if (rc == DRV_SUCCESS)
        rv.haveNew = true;
    else
        CHECK(rc, " in GetNumberNewImages");

    rv.first -= 1;
    rv.second -= 1;

    EXIT("GetNumberNewImages");
    return rv;
}

void AndorCamera::SetAcquisitionMode(AcquisitionMode mode) throw(Error) {
    ENTRY("SetAcquisitionMode");
    int armor[200];
    initArmor(armor, 200);
    clearStack(armor);
    CHECK( ::SetAcquisitionMode(int(mode)), " in SetAcquisitionMode" );
    EXIT("SetAcquisitionMode");
}

void AndorCamera::SetNumberKinetics( int number ) throw(Error) {
    ENTRY("SetNumberKinetics");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetNumberKinetics(number), " in SetNumberKinetics" );
    EXIT("SetNumberKinetics");
}

void AndorCamera::SetKineticCycleTime( float number ) throw(Error) {
    ENTRY("SetKineticCycleTime");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetKineticCycleTime(number), " in SetKineticCycleTime" );
    EXIT("SetKineticCycleTime");
}

void AndorCamera::SetExposureTime( float number ) throw(Error) {
    ENTRY("SetExposureTime");
    int armor[20];
    initArmor(armor, 20);
    clearStack(armor);
    CHECK( ::SetExposureTime(number), " in SetExposureTime" );
    EXIT("SetExposureTime");
}

float AndorCamera::GetExposureTime() throw(Error) {
    ENTRY("GetExposureTime");
    float armor[100];
    initArmor(armor, 100);
    clearStack(armor);
    CHECK( ::GetAcquisitionTimings( &armor[79], &armor[80], &armor[81] ),
           " in GetExposureTime" );
    EXIT("GetExposureTime");
    return armor[19];
}

float AndorCamera::GetAccumulationCycleTime() throw(Error) {
    ENTRY("GetAccumulationCycleTime");
    float armor[40];
    clearStack(armor);
    CHECK( ::GetAcquisitionTimings( &armor[19], &armor[20], &armor[21] ),
           " in GetAccumulationCycleTime" );
    EXIT("GetAccumulationCycleTime");
    return armor[20];
}

float AndorCamera::GetKineticCycleTime() throw(Error) {
    ENTRY("GetKineticCycleTime");
    float armor[40];
    clearStack(armor);
    CHECK( ::GetAcquisitionTimings( &armor[19], &armor[20], &armor[21] ),
           " in GetKineticCycleTime" );
    EXIT("GetKineticCycleTime");
    return armor[21];
}

void AndorCamera::StartAcquisition() throw(Error) {
    ENTRY("StartAcquisition");
    int armor[10];
    armor[2] = 0;
    CHECK( ::StartAcquisition(), " in StartAcquisition" );
    EXIT("StartAcquisition");
}

static void realGetImages16(long int fi, long int li, WORD* buf,
    unsigned long bufSize, long int& fvi, long int& lvi) throw(Error)
{
    int armor[100];
    clearStack(armor);
    CHECK(::GetImages16(fi, li, buf, bufSize, &fvi, &lvi), 
          " in GetImages16");
}

void AndorCamera::GetImages16
    (AndorCamera::Range range, uint16_t *buf,
                unsigned long bufSize, Range &read) throw(Error)
{
    assert( sizeof(WORD) == sizeof(uint16_t) );

    ENTRY("GetImages16");
    realGetImages16(range.first+1, range.second+1, buf, bufSize, 
        read.first, read.second);
    read.first -= 1;
    read.second -= 1;
    EXIT("GetImages16");
}
