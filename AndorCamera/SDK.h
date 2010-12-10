/** \file SDK.h
 *
 *  Wrappers around functions of the Andor Software Development Kit.
 *  These functions are, for the most part, undocumented because they
 *  are well defined in the Andor SDK documentation.
 */
#ifndef ANDORCAMERA_SDK_H
#define ANDORCAMERA_SDK_H

/* This sounds ridiculous, but, for some reason, these includes
 * are necessary for linking the ATMCD32D.H with mingw. */
#include <stdexcept>
#include <windows.h>
#undef __dllspec
#define __dllspec(x)
#undef __declspec
#define __declspec(x)
#undef WINAPI
#define WINAPI
#include <ATMCD32D.H>

#include <stdint.h>
#include <AndorCamera/ControlReference.h>
#include "Error.h"

#include "SDK_Constants.h"

namespace AndorCamera {
    /** An image number. 
     *
     *  \attention The image number is, different
     *  from the Andor SDK, counted from 0. */
    typedef long int ImageNumber;
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
    void checkAndorCode( int code, int line ) throw(Error);

    long GetAvailableCameras() throw(Error);
    /** Measure CCD temperature.
     *  \return If the temperature stabilized,
     *          the bool part of this pair will be true, false otherwise.
     *          The second member is the temperature in degrees Celcius.
     */
    std::pair<bool,float> GetTemperatureF() throw(Error);

    void  SetADChannel( ADChannel index ) throw(Error);
    int   GetBitDepth( ADChannel index ) throw(Error);

    int   GetNumberHSSpeeds( ADChannel c, OutputAmp o ) throw(Error);
    float GetVSSpeed( int index ) throw(Error);
    float GetHSSpeed( ADChannel c, OutputAmp o, int index ) throw(Error);
    void  SetVSSpeed( int index ) throw(Error);
    void  SetHSSpeed( int amplification, int index ) throw(Error);

    /** Get size of CCD in pixels.
     * \return Width (first member) and height of detector. */
    std::pair<int,int> GetDetector() throw(Error);

    bool  IsInternalMechanicalShutter() throw(Error);
    void SetShutter( int highety, int mode, int op_time, int cl_time )
        throw(Error);

    void SetImage( int binx, int biny, 
                   int firstCol, int lastCol, int firstRow, int lastRow )
        throw(Error);
    void SetImageNoBinning(int firstCol, int lastCol, 
                           int firstRow, int lastRow ) throw(Error);

    enum AcquisitionMode { Image = 1, Kinetics = 3 };
    void SetAcquisitionMode( AcquisitionMode mode ) throw(Error);

    void SetNumberKinetics( int number ) throw(Error);

    void SetKineticCycleTime( float time ) throw(Error);
    void SetExposureTime( float time ) throw(Error);
    float GetExposureTime() throw(Error);
    float GetAccumulationCycleTime() throw(Error);
    float GetKineticCycleTime() throw(Error);

    void StartAcquisition() throw(Error);

    NewImages GetNumberNewImages() throw(Error);
    NewImages GetNumberAvailableImages() throw(Error);

    void GetImages16
        (Range toRead, uint16_t *buf, unsigned long bufSize, Range &read) 
        throw(Error);

}

#endif
