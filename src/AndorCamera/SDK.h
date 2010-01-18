/** \file SDK.h
 *
 *  Wrappers around functions of the Andor Software Development Kit.
 *  These functions are, for the most part, undocumented because they
 *  are well defined in the Andor SDK documentation.
 */
#ifndef ANDORCAMERA_SDK_H
#define ANDORCAMERA_SDK_H

#include <stdint.h>
#include "Error.h"
#include "Config.h"

#define CHECK_ANDOR(x) { int line = __LINE__; checkAndorCode( x, line ); }

namespace SDK {
    using AndorCamera::Error;
    using AndorCamera::ADChannel;
    using AndorCamera::OutputAmp;

    /** An image number. 
     *
     *  \attention The image number is, different
     *  from the Andor SDK, counted from 0. */
    typedef unsigned long int ImageNumber;
    /** A range of image numbers. 
     *
     *  \attention The numbers are, different
     *  from the Andor SDK, counted from 0. */
    typedef std::pair<ImageNumber,ImageNumber> Range;
    struct NewImages : public Range { bool haveNew; };

    /** Throw an error if \a code is not DRV_SUCCESS.
     *  \param code An Andor return code
     *  \param line The source line number the error occured in
     **/
    void checkAndorCode( int code, int line );

    long GetAvailableCameras();
    /** Measure CCD temperature.
     *  \return If the temperature stabilized,
     *          the bool part of this pair will be true, false otherwise.
     *          The second member is the temperature in degrees Celcius.
     */
    std::pair<bool,float> GetTemperatureF();

    int   GetNumberADChannels();
    void  SetADChannel( ADChannel index );
    int   GetBitDepth( ADChannel index );

    int   GetNumberVSSpeeds();
    float GetVSSpeed( int index );
    void  SetVSSpeed( int index );
    int   GetNumberHSSpeeds( ADChannel c, OutputAmp o );
    float GetHSSpeed( ADChannel c, OutputAmp o, int index );
    void  SetHSSpeed( int amplification, int index );

    /** Get size of CCD in pixels.
     * \return Width (first member) and height of detector. */
    std::pair<int,int> GetDetector();

    bool  IsInternalMechanicalShutter();

    void SetImage( int binx, int biny, 
                   int firstCol, int lastCol, int firstRow, int lastRow )
;
    void SetImageNoBinning(int firstCol, int lastCol, 
                           int firstRow, int lastRow );

    void SetReadMode( AndorCamera::ReadoutMode mode );

    void SetAcquisitionMode( AndorCamera::AcquisitionMode mode );

    void SetNumberKinetics( int number );

    void SetKineticCycleTime( float time );
    void SetExposureTime( float time );
    float GetExposureTime();
    float GetAccumulationCycleTime();
    float GetKineticCycleTime();

    void StartAcquisition();

    NewImages GetNumberNewImages();
    NewImages GetNumberAvailableImages();

    enum AcquisitionState { New_Images, No_New_Images, Missed_Images };
    AcquisitionState WaitForAcquisition();

    enum CameraState { Idle, Temperature_Cycle, Is_Acquiring, 
                       Accum_time_not_met, Kinetic_time_not_met,
                       Error_ACK, Acq_Buffer, Spool_Error };
    CameraState GetStatus();

    void FreeInternalMemory();
    /** @return true if an acquisition was aborted, false if DRV_IDLE occured. */
    bool AbortAcquisition();
    void Initialize(std::string init_file_dir);

    AcquisitionState GetImages16
        (Range toRead, uint16_t *buf, unsigned long bufSize, Range &read);

    void SetTemperature(int temperature);
    void CoolerON();
    void CoolerOFF();

    std::pair<int,int> GetTemperatureRange();

    typedef long CameraHandle;
    CameraHandle GetCameraHandle(int camera_index);
    void SetCurrentCamera(CameraHandle handle);
    void ShutDown();

    namespace Shutter {
        enum TTL { High = 0, Low = 1 };
        enum Mode { Auto = 0, Open = 1, Closed = 2 };
    };
    void SetShutter( Shutter::TTL ttl, Shutter::Mode mode, 
                     int open_time, int close_time );

    namespace Trigger {
        enum Mode { 
            Internal = 0, 
            External = 1,
            External_Start = 6,
            External_Exposure = 7,
            External_FVB_EM = 9,
            Software_Trigger
        };
    };
    void SetTriggerMode( Trigger::Mode mode );

    void SetFrameTransferMode( bool active );
    void SetEMCCDGain( int gain );
    
    std::pair<int,int> GetEMCCDGainRange();
}

#endif
