#include "ATMCD32D.H"
#include "dummycamera.h"
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <unistd.h>

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

namespace dummycamera {
    int cameraCount = 2;
    int currentCam = 0;
    float surroundTemperature = 20;
    double coolingRate = 0.1;

    Camera cameras[8];
    Camera *curCam = cameras;
}

bool operator<(const struct timeval& a, const struct timeval& b)
 { return a.tv_sec < b.tv_sec || ((a.tv_sec == b.tv_sec) && a.tv_usec < b.tv_usec); }
bool operator>(const struct timeval& a, const struct timeval& b)
 { return a.tv_sec > b.tv_sec || ((a.tv_sec == b.tv_sec) && a.tv_usec > b.tv_usec); }

struct timeval operator+(const struct timeval& a, double time) 
{
    struct timeval rv = a;
    rv.tv_sec += int(time);
    unsigned long microseconds 
        = rv.tv_usec + (int)round( (time - int(time)) * 1E6 );
    while ( microseconds > 1E6 ) {
        rv.tv_sec++;
        microseconds -= 1E6;
    }
    rv.tv_usec = microseconds;
    return rv;
}

struct timeval operator-(const struct timeval& a, double time) 
{
    struct timeval rv = a;
    rv.tv_sec -= int(time);
    long microseconds 
        = rv.tv_usec - (int)round( (time - int(time)) * 1E6 );
    while ( microseconds < 0 ) {
        rv.tv_sec--;
        microseconds += 1E6;
    }
    rv.tv_usec = microseconds;
    return rv;
}

struct timeval operator-(const struct timeval& a, const struct timeval& b) 
{
    struct timeval rv;
    rv.tv_sec = a.tv_sec - b.tv_sec;
    long microseconds = long(a.tv_usec) - long(b.tv_usec);
    while (microseconds < 0) {
        rv.tv_sec --;
        microseconds += 1E6;
    }
    rv.tv_usec = microseconds;
    return rv;
}

std::ostream& operator<<(std::ostream& o, const struct timeval& a) 
{
    return (o << a.tv_sec << "." << std::setw(6) << std::setfill('0') << a.tv_usec << std::setfill(' '));
}

using namespace dummycamera;

bool cam_is_initialized() throw() { 
    return (currentCam >= 0 && currentCam < cameraCount &&
            cameras[currentCam].cameraInitialized);
}

unsigned int GetStatus(int* status) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) curCam->update_ringbuffer();
    *status = (curCam->acquiring) ? DRV_ACQUIRING : DRV_IDLE;
    return DRV_SUCCESS;
}

unsigned int GetAvailableCameras(long* totalCameras) {
    *totalCameras = cameraCount;
    return DRV_SUCCESS;
}

unsigned int GetTemperatureF(float* target) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    float toTemp =  (curCam->coolerOn) 
        ? curCam->desiredTemperature : surroundTemperature;
    struct timeval elapsed;
    gettimeofday(&elapsed, NULL);
    elapsed = elapsed - curCam->coolerSwitch;
    double elapsed_s = elapsed.tv_sec + elapsed.tv_usec * 1E-6;

    *target = curCam->lastTemperature + 
        ( toTemp - curCam->lastTemperature )
            * (1 - exp( - coolingRate * elapsed_s ));

    if ( abs( *target - toTemp ) < 3 )
        return DRV_TEMP_STABILIZED;
    else
        return DRV_TEMP_NOT_REACHED;
}

unsigned int SetADChannel(int i) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->ad_channels_with_hsspeeds.size()) <= i )
        return DRV_P1INVALID;

    curCam->adChannel = i;

    return DRV_SUCCESS;
}
unsigned int GetBitDepth(int chan, int* depth){
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->ad_channels_with_hsspeeds.size()) <= chan )
        return DRV_P1INVALID;

    *depth = curCam->ad_channels_with_hsspeeds[chan].first;

    return DRV_SUCCESS;
}
unsigned int GetNumberADChannels(int* i) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

     *i = curCam->ad_channels_with_hsspeeds.size();

    return DRV_SUCCESS;
}
unsigned int GetNumberHSSpeeds(int chan, int , int* to) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->ad_channels_with_hsspeeds.size()) <= chan )
        return DRV_P1INVALID;

    *to = curCam->ad_channels_with_hsspeeds[chan].second.size();

    return DRV_SUCCESS;
}
unsigned int GetNumberVSSpeeds(int* num) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    *num = curCam->vsspeeds.size();

    return DRV_SUCCESS;
}
unsigned int GetHSSpeed(int chan, int , int speed, float* to) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->ad_channels_with_hsspeeds.size()) <= chan )
        return DRV_P1INVALID;
    else if ( int(curCam->ad_channels_with_hsspeeds[chan].second.size()) 
                <= speed) return DRV_P3INVALID;

    *to = curCam->ad_channels_with_hsspeeds[chan].second[speed];

    return DRV_SUCCESS;
}
unsigned int GetVSSpeed(int index, float* to) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    if ( int(curCam->vsspeeds.size()) <= index ) return DRV_P1INVALID;

    *to = curCam->vsspeeds[index];

    return DRV_SUCCESS;
}
unsigned int SetVSSpeed(int to) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->vsspeeds.size()) <= to ) return DRV_P1INVALID;

    curCam->vsspeed = to;

    return DRV_SUCCESS;
}
unsigned int SetHSSpeed(int , int to) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;
    if ( int(curCam->ad_channels_with_hsspeeds[curCam->adChannel]
            .second.size()) <= to ) return DRV_P1INVALID;

    curCam->hsspeed = to;

    return DRV_SUCCESS;
}
unsigned int GetDetector(int* w, int* h) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    *w = curCam->detectorWidth;
    *h = curCam->detectorHeight;

    return DRV_SUCCESS;
}
unsigned int IsInternalMechanicalShutter(int *is) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    *is = 1;

    return DRV_SUCCESS;
}
unsigned int SetImage(int , int , int l , int r, int t, int b) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    curCam->left = l;
    curCam->right = r;
    curCam->top = t;
    curCam->bottom = b;

    return DRV_SUCCESS;
}
unsigned int GetNumberAvailableImages (at_32* first, at_32* last) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;

    curCam->update_ringbuffer();

    if ( curCam->ringbuffer_end >= curCam->ringbuffer_start ) {
        *first = curCam->ringbuffer_start;
        *last = curCam->ringbuffer_end;
        return DRV_SUCCESS;
    } else
        return DRV_NO_NEW_DATA;
}
unsigned int GetNumberNewImages (long* first, long* last) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;

    curCam->update_ringbuffer();

    if ( curCam->ringbuffer_end >= curCam->ringbuffer_mark ) {
        *first = curCam->ringbuffer_mark;
        *last = curCam->ringbuffer_end;
        return DRV_SUCCESS;
    } else
        return DRV_NO_NEW_DATA;
}

void Camera::update_ringbuffer() {
    struct timeval start_of_buffer = 
        start_of_acquisition + kin_time * ringbuffer_end;

    struct timeval image_start = start_of_buffer;

    struct timeval now;
    gettimeofday(&now, NULL);
    while ( (kinetic_images == -1 || ringbuffer_end < kinetic_images) && 
            image_start + acc_time * accumulations < now ) 
    {
        image_start = image_start + kin_time;
        ringbuffer_end++;
    }

    if ( ringbuffer_end == kinetic_images && 
         image_start + acc_time * accumulations < now )
    {
        acquiring = false;
    }

    ringbuffer_start = std::max(ringbuffer_start, 
        ringbuffer_end - ring_buffer_size + 1);
    ringbuffer_mark = std::max( ringbuffer_mark, ringbuffer_start );
}

unsigned int SetAcquisitionMode(int mode) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    if ( mode != 1 && mode != 3 && mode != 5 )
        std::cerr << "Acquisition mode " << mode << " not supported.\n";

    if ( mode == 1 ) 
        curCam->kinetic_images = 1;
    else if ( mode == 5 ) {
        curCam->kinetic_images = -1;
    }

    return DRV_SUCCESS;
}
unsigned int SetNumberKinetics(int num) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    curCam->kinetic_images = num;

    return DRV_SUCCESS;
}
unsigned int SetKineticCycleTime(float f) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    curCam->kin_time = std::max<double>( f, curCam->min_kin_time() );

    return DRV_SUCCESS;
}

unsigned int SetExposureTime(float exp_time) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    curCam->exp_time = std::max<double>( exp_time,curCam->min_exp_time() );
    curCam->acc_time = std::max<double>
        ( curCam->acc_time,curCam->min_acc_time() );
    curCam->kin_time = std::max<double>
        ( curCam->kin_time,curCam->min_kin_time() );

    return DRV_SUCCESS;
}

unsigned int GetAcquisitionTimings(float* e, float* a, float* k) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    *e = curCam->exp_time;
    *a = curCam->acc_time;
    *k = curCam->kin_time;

    return DRV_SUCCESS;
}

unsigned int StartAcquisition(void) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    curCam->acquiring = true;
    gettimeofday( &curCam->start_of_acquisition, NULL );

    curCam->ringbuffer_start = 1;
    curCam->ringbuffer_mark = 1;
    curCam->ringbuffer_end = 0;

    return DRV_SUCCESS;
}

unsigned int SetShutter(int , int , int , int ) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    if ( curCam->acquiring ) return DRV_ACQUIRING;

    return DRV_SUCCESS;
}

unsigned short dummycamera::pixelValue( int x, int y, int z ) {
    return ((x+y) * std::min( curCam->gain, 9 ) + z) % 10;
}

unsigned int GetImages16 (
    long first, long last, WORD* buffer,
    unsigned long bufSize, 
    long* validfirst, long* validlast) 
{
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;

    int32_t first_av, last_av;
    unsigned long rv = GetNumberAvailableImages(&first_av, &last_av);
    if ( rv == DRV_NO_NEW_DATA ) return rv;

    if ( first > curCam->ringbuffer_mark ) {
        std::cerr << "Dummy camera can only deliver images in sequence, but you "
                     "asked for image " << first << " before " << first_av << 
                     " was delivered. Am assuming now you asked for " << first << ", too.\n";
        first = first_av;
    }
    first = std::max<long>( first, first_av );
    last = std::min<long>( last, last_av );

    if ( first > last )
        return DRV_NO_NEW_DATA;
    else if ( int(bufSize) < ( curCam->acquisitionAreaSize() * (last-first+1)))
        return DRV_P4INVALID;
    else {
        *validfirst = first;
        *validlast = last;

        std::cerr << "Getting images " << first << " " << last << " with size "
                  << curCam->height() << " " << curCam->width() << "\n";
        std::cerr << "Pixel map: " << buffer << " " << buffer+ 
            (last-first+1) * curCam->acquisitionAreaSize() -1 << "\n";
        for (int i = first; i <= last; i++)
            for (int y = 0; y < curCam->height(); y++)
                for (int x = 0; x < curCam->width(); x++) {
                    buffer[ curCam->acquisitionAreaSize() * (i-first) 
                            + y * curCam->width() + x ]
                        = pixelValue( x, y, i );
                }

        curCam->ringbuffer_mark = last+1;
        return DRV_SUCCESS;
    }
}

unsigned int Initialize (char *) {
    curCam->cameraInitialized = true;
    return DRV_SUCCESS;
}

unsigned int FreeInternalMemory() {
    return DRV_SUCCESS;
}

unsigned int AbortAcquisition() {
    curCam->acquiring = false;
    return DRV_SUCCESS;
}

unsigned int CoolerOFF(void) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    GetTemperatureF(&curCam->lastTemperature);
    curCam->coolerOn = false;
    curCam->coolerSwitch = time(NULL);
    return DRV_SUCCESS;
}
unsigned int CoolerON(void) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    GetTemperatureF(&curCam->lastTemperature);
    curCam->coolerOn = true;
    curCam->coolerSwitch = time(NULL);
    return DRV_SUCCESS;
}
unsigned int SetTemperature(int temperature) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    curCam->desiredTemperatureSet = true;
    curCam->desiredTemperature = temperature;
    return DRV_SUCCESS;
}

unsigned int GetTemperatureRange(int* mintemp,int* maxtemp) {
    if ( !cam_is_initialized() ) return DRV_NOT_INITIALIZED;
    *mintemp = -100;
    *maxtemp = 30;
    return DRV_SUCCESS;
}
unsigned int GetCameraHandle(long cameraIndex, long* cameraHandle) 
{
    if ( cameraIndex >= cameraCount ) return DRV_P1INVALID;
    *cameraHandle = cameraIndex + 42;
    return DRV_SUCCESS;
}
unsigned int SetCurrentCamera(long cameraHandle) {
    if ( cameraHandle - 42 < 0 || cameraHandle - 42 >= cameraCount )
        return DRV_P1INVALID;

    currentCam = cameraHandle - 42;
    curCam = cameras + currentCam;
    return DRV_SUCCESS;
}
unsigned int ShutDown() {
    curCam->cameraInitialized = false;
    return DRV_SUCCESS;
}

unsigned int WaitForAcquisition(void) {
    double acquisition_length =
            curCam->acc_time * curCam->accumulations;
    int image = 0;

    struct timeval now;
    gettimeofday(&now, NULL);

    while ( curCam->start_of_acquisition + acquisition_length < now && 
            ( curCam->kinetic_images == -1 || image < curCam->kinetic_images) )
    {
        image++;
        acquisition_length += curCam->kin_time;
    }
    if ( curCam->start_of_acquisition + acquisition_length > now ) {
        struct timeval wait_time = (curCam->start_of_acquisition + acquisition_length) - now;
#ifdef HAVE_USLEEP
        usleep(wait_time.tv_sec * 1E6 + wait_time.tv_usec );
#else
#ifdef HAVE_WINDOWS_H
        int rounder = (wait_time.tv_usec % 1000 == 0) ? 0 : 1;
        Sleep( wait_time.tv_sec * 1E3 + wait_time.tv_usec / 1000 
               + rounder );
#endif
#endif
        return DRV_SUCCESS;
    } else
        return DRV_IDLE;
}

unsigned int SetTriggerMode(int) {
    return DRV_SUCCESS;
}
unsigned int SetFrameTransferMode(int) {
    return DRV_SUCCESS;
}
unsigned int SetEMCCDGain(int gain) {
    curCam->gain = gain;
    return DRV_SUCCESS;
}
unsigned int SetReadMode(int) {
    return DRV_SUCCESS;
}

unsigned int GetEMGainRange(int *low, int *high) {
    *low =-100; *high = 30;
    return DRV_SUCCESS;
}
