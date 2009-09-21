#include <stdint.h>

#define DRV_DATATYPE 20064
#define DRV_DRIVER_ERRORS 20065
#define DRV_P1INVALID 20066
#define DRV_P2INVALID 20067
#define DRV_P3INVALID 20068
#define DRV_P4INVALID 20069
#define DRV_INIERROR 20070
#define DRV_COFERROR 20071
#define DRV_ACQUIRING 20072
#define DRV_IDLE 20073
#define DRV_TEMPCYCLE 20074
#define DRV_NOT_INITIALIZED 20075
#define DRV_P5INVALID 20076
#define DRV_P6INVALID 20077
#define DRV_INVALID_MODE 20078
#define DRV_INVALID_FILTER 20079

#define DRV_ERROR_CODES 20001
#define DRV_SUCCESS 20002
#define DRV_VXDNOTINSTALLED 20003
#define DRV_ERROR_SCAN 20004
#define DRV_ERROR_CHECK_SUM 20005
#define DRV_ERROR_FILELOAD 20006
#define DRV_UNKNOWN_FUNCTION 20007
#define DRV_ERROR_VXD_INIT 20008
#define DRV_ERROR_ADDRESS 20009
#define DRV_ERROR_PAGELOCK 20010
#define DRV_ERROR_PAGEUNLOCK 20011
#define DRV_ERROR_BOARDTEST 20012
#define DRV_ERROR_ACK 20013
#define DRV_ERROR_UP_FIFO 20014
#define DRV_ERROR_PATTERN 20015

#define DRV_ACQUISITION_ERRORS 20017
#define DRV_ACQ_BUFFER 20018
#define DRV_ACQ_DOWNFIFO_FULL 20019
#define DRV_PROC_UNKONWN_INSTRUCTION 20020
#define DRV_ILLEGAL_OP_CODE 20021
#define DRV_KINETIC_TIME_NOT_MET 20022
#define DRV_ACCUM_TIME_NOT_MET 20023
#define DRV_NO_NEW_DATA 20024
#define DRV_PCI_DMA_FAIL	20025
#define DRV_SPOOLERROR 20026
#define DRV_SPOOLSETUPERROR 20027
#define DRV_FILESIZELIMITERROR 20028
#define DRV_ERROR_FILESAVE 20029

#define DRV_TEMPERATURE_CODES 20033
#define DRV_TEMPERATURE_OFF 20034
#define DRV_TEMPERATURE_NOT_STABILIZED 20035
#define DRV_TEMPERATURE_STABILIZED 20036
#define DRV_TEMPERATURE_NOT_REACHED 20037
#define DRV_TEMPERATURE_OUT_RANGE 20038
#define DRV_TEMPERATURE_NOT_SUPPORTED 20039
#define DRV_TEMPERATURE_DRIFT 20040
#define DRV_NO_NEW_DATA 20024

#define DRV_TEMP_CODES 20033
#define DRV_TEMP_OFF 20034
#define DRV_TEMP_NOT_STABILIZED 20035
#define DRV_TEMP_STABILIZED 20036
#define DRV_TEMP_NOT_REACHED 20037
#define DRV_TEMP_OUT_RANGE 20038
#define DRV_TEMP_NOT_SUPPORTED 20039
#define DRV_TEMP_DRIFT 20040

#define at_32 int32_t
#define WORD uint16_t

#define EXPNETTYPE
#define WINAPI

EXPNETTYPE unsigned int WINAPI GetStatus(int* status);
EXPNETTYPE unsigned int WINAPI GetAvailableCameras(long* totalCameras);
EXPNETTYPE unsigned int WINAPI GetTemperatureF(float* temperature);
EXPNETTYPE unsigned int WINAPI SetADChannel(int channel);
EXPNETTYPE unsigned int WINAPI GetBitDepth(int channel, int* depth);
EXPNETTYPE unsigned int WINAPI GetNumberADChannels(int* channels);
EXPNETTYPE unsigned int WINAPI GetNumberHSSpeeds(int channel, int type, int* speeds);
EXPNETTYPE unsigned int WINAPI GetNumberVSSpeeds(int* speeds);
EXPNETTYPE unsigned int WINAPI GetHSSpeed(int channel, int type, int index, float* speed);
EXPNETTYPE unsigned int WINAPI GetVSSpeed(int index, float* speed);
EXPNETTYPE unsigned int WINAPI SetVSSpeed(int index);
EXPNETTYPE unsigned int WINAPI SetHSSpeed(int type, int index);
EXPNETTYPE unsigned int WINAPI GetDetector(int* xpixels, int* ypixels);
EXPNETTYPE unsigned int WINAPI IsInternalMechanicalShutter(int *InternalShutter);
EXPNETTYPE unsigned int WINAPI SetImage(int hbin, int vbin, int hstart, int hend, int vstart, int vend);
EXPNETTYPE unsigned int WINAPI GetNumberAvailableImages (at_32* first, at_32* last);
EXPNETTYPE unsigned int WINAPI GetNumberNewImages (long* first, long* last);
EXPNETTYPE unsigned int WINAPI SetAcquisitionMode(int mode);
EXPNETTYPE unsigned int WINAPI SetNumberKinetics(int number);
EXPNETTYPE unsigned int WINAPI SetKineticCycleTime(float time);
EXPNETTYPE unsigned int WINAPI SetExposureTime(float time);
EXPNETTYPE unsigned int WINAPI GetAcquisitionTimings(float* exposure, float* accumulate, float* kinetic);
EXPNETTYPE unsigned int WINAPI StartAcquisition(void);
EXPNETTYPE unsigned int WINAPI SetShutter(int type, int mode, int closingtime, int openingtime);
EXPNETTYPE unsigned int WINAPI GetImages16 (long first, long last, WORD* array, unsigned long size, long* validfirst, long* validlast);
EXPNETTYPE unsigned int WINAPI Initialize (char *directory);
EXPNETTYPE unsigned int WINAPI FreeInternalMemory(void);
EXPNETTYPE unsigned int WINAPI AbortAcquisition(void);
EXPNETTYPE unsigned int WINAPI CoolerOFF(void);
EXPNETTYPE unsigned int WINAPI CoolerON(void);
EXPNETTYPE unsigned int WINAPI SetTemperature(int temperature);

EXPNETTYPE unsigned int WINAPI GetTemperatureRange(int* mintemp,int* maxtemp);
EXPNETTYPE unsigned int WINAPI GetCameraHandle(long cameraIndex, long* cameraHandle);
EXPNETTYPE unsigned int WINAPI SetCurrentCamera(long cameraHandle);
EXPNETTYPE unsigned int WINAPI ShutDown(void);
EXPNETTYPE unsigned int WINAPI WaitForAcquisition(void);
EXPNETTYPE unsigned int WINAPI SetTriggerMode(int mode);
EXPNETTYPE unsigned int WINAPI SetFrameTransferMode(int mode);
EXPNETTYPE unsigned int WINAPI SetEMCCDGain(int gain);
EXPNETTYPE unsigned int WINAPI SetReadMode(int mode);
EXPNETTYPE unsigned int WINAPI GetEMGainRange(int *low, int *high);
