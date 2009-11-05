#define ANDORCAMERA_ERROR_CPP
#include "Error.h"
#include "SDK.h"
#include <iostream>
#include <dStorm/helpers/thread.h>
#include <stdarg.h>
#include <stdio.h>

using namespace std;
using namespace AndorCamera;

/** Return a string defining the Andor return code.
 *  \param return_code A return code from an Andor function
 *  \param locSpec     If \a return_code is an invalid code,
 *                     an error is thrown. In that case, locSpec
 *                     will be included in the exception. */
static const char *str4andor(long return_code, const char *locSpec)
 
{
    const char *str = NULL;
    switch (return_code) {
        case 20001: str = "DRV_ERROR_CODES"; break;
        case 20002: str = "DRV_SUCCESS"; break;
        case 20003: str = "DRV_VXDNOTINSTALLED"; break;
        case 20004: str = "DRV_ERROR_SCAN"; break;
        case 20005: str = "DRV_ERROR_CHECK_SUM"; break;
        case 20006: str = "DRV_ERROR_FILELOAD"; break;
        case 20007: str = "DRV_UNKNOWN_FUNCTION"; break;
        case 20008: str = "DRV_ERROR_VXD_INIT"; break;
        case 20009: str = "DRV_ERROR_ADDRESS"; break;
        case 20010: str = "DRV_ERROR_PAGELOCK"; break;
        case 20011: str = "DRV_ERROR_PAGEUNLOCK"; break;
        case 20012: str = "DRV_ERROR_BOARDTEST"; break;
        case 20013: str = "DRV_ERROR_ACK"; break;
        case 20014: str = "DRV_ERROR_UP_FIFO"; break;
        case 20015: str = "DRV_ERROR_PATTERN"; break;
        case 20017: str = "DRV_ACQUISITION_ERRORS"; break;
        case 20018: str = "DRV_ACQ_BUFFER"; break;
        case 20019: str = "DRV_ACQ_DOWNFIFO_FULL"; break;
        case 20020: str = "DRV_PROC_UNKONWN_INSTRUCTION"; break;
        case 20021: str = "DRV_ILLEGAL_OP_CODE"; break;
        case 20022: str = "DRV_KINETIC_TIME_NOT_MET"; break;
        case 20023: str = "DRV_ACCUM_TIME_NOT_MET"; break;
        case 20024: str = "DRV_NO_NEW_DATA"; break;
        case 20025: str = "DRV_PCI_DMA_FAIL"; break;
        case 20026: str = "DRV_SPOOLERROR"; break;
        case 20027: str = "DRV_SPOOLSETUPERROR"; break;
        case 20028: str = "DRV_FILESIZELIMITERROR"; break;
        case 20029: str = "DRV_ERROR_FILESAVE"; break;
        case 20033: str = "DRV_TEMPERATURE_CODES"; break;
        case 20034: str = "DRV_TEMPERATURE_OFF"; break;
        case 20035: str = "DRV_TEMPERATURE_NOT_STABILIZED"; break;
        case 20036: str = "DRV_TEMPERATURE_STABILIZED"; break;
        case 20037: str = "DRV_TEMPERATURE_NOT_REACHED"; break;
        case 20038: str = "DRV_TEMPERATURE_OUT_RANGE"; break;
        case 20039: str = "DRV_TEMPERATURE_NOT_SUPPORTED"; break;
        case 20040: str = "DRV_TEMPERATURE_DRIFT"; break;
        case 20049: str = "DRV_GENERAL_ERRORS"; break;
        case 20050: str = "DRV_INVALID_AUX"; break;
        case 20051: str = "DRV_COF_NOTLOADED"; break;
        case 20052: str = "DRV_FPGAPROG"; break;
        case 20053: str = "DRV_FLEXERROR"; break;
        case 20054: str = "DRV_GPIBERROR"; break;
        case 20055: str = "DRV_EEPROMVERSIONERROR"; break;
        case 20064: str = "DRV_DATATYPE"; break;
        case 20065: str = "DRV_DRIVER_ERRORS"; break;
        case 20066: str = "DRV_P1INVALID"; break;
        case 20067: str = "DRV_P2INVALID"; break;
        case 20068: str = "DRV_P3INVALID"; break;
        case 20069: str = "DRV_P4INVALID"; break;
        case 20070: str = "DRV_INIERROR"; break;
        case 20071: str = "DRV_COFERROR"; break;
        case 20072: str = "DRV_ACQUIRING"; break;
        case 20073: str = "DRV_IDLE"; break;
        case 20074: str = "DRV_TEMPCYCLE"; break;
        case 20075: str = "DRV_NOT_INITIALIZED"; break;
        case 20076: str = "DRV_P5INVALID"; break;
        case 20077: str = "DRV_P6INVALID"; break;
        case 20078: str = "DRV_INVALID_MODE"; break;
        case 20079: str = "DRV_INVALID_FILTER"; break;
        case 20080: str = "DRV_I2CERRORS"; break;
        case 20081: str = "DRV_I2CDEVNOTFOUND"; break;
        case 20082: str = "DRV_I2CTIMEOUT"; break;
        case 20083: str = "DRV_P7INVALID"; break;
        case 20089: str = "DRV_USBERROR"; break;
        case 20090: str = "DRV_IOCERROR"; break;
        case 20091: str = "DRV_VRMVERSIONERROR"; break;
        case 20093: str = "DRV_USB_INTERRUPT_ENDPOINT_ERROR"; break;
        case 20094: str = "DRV_RANDOM_TRACK_ERROR"; break;
        case 20095: str = "DRV_INVALID_TRIGGER_MODE"; break;
        case 20096: str = "DRV_LOAD_FIRMWARE_ERROR"; break;
        case 20097: str = "DRV_DIVIDE_BY_ZERO_ERROR"; break;
        case 20098: str = "DRV_INVALID_RINGEXPOSURES"; break;
        case 20115: str = "DRV_ERROR_MAP"; break;
        case 20116: str = "DRV_ERROR_UNMAP"; break;
        case 20117: str = "DRV_ERROR_MDL"; break;
        case 20118: str = "DRV_ERROR_UNMDL"; break;
        case 20119: str = "DRV_ERROR_BUFFSIZE"; break;
        case 20121: str = "DRV_ERROR_NOHANDLE"; break;
        case 20130: str = "DRV_GATING_NOT_AVAILABLE"; break;
        case 20131: str = "DRV_FPGA_VOLTAGE_ERROR"; break;
        case 20156: str = "DRV_MSTIMINGS_ERROR"; break;
        case 20990: str = "DRV_ERROR_NOCAMERA"; break;
        case 20991: str = "DRV_NOT_SUPPORTED"; break;
        case 20992: str = "DRV_NOT_AVAILABLE"; break;
        default:    throw Error("Encountered unknown Andor error code"
                             + ((locSpec) ? (" in " + string(locSpec)) 
                                         : string(""))
                             + string(": "));
    }
    return str;
}

/* See AndorCamera/Error.h */
AndorCamera::Error::Error(unsigned int andor_code, const char *locSpec, 
            const std::string& message)
: andorReturnCode(andor_code), error_message("")
{
    const char *code = str4andor(andor_code, locSpec);
    error_message = string(code) + locSpec;
    if (error_message != "" && message != "")
        error_message += ": ";
    error_message += message;
    _what = error_message.c_str();
    cerr << "Andor error: " << _what << endl;
}

/* See AndorCamera/Error.h */
Error::Error(const std::string& message, ...)
: andorReturnCode(0), error_message("")
{
    STATUS("Constructing Andor camera error");
    va_list argptr;
    char buffer[1024];
    va_start( argptr, message );
    LOCKING("Printing " << message.c_str());
    vsnprintf(buffer, 1024, message.c_str(), argptr);
    LOCKING("Printed " << buffer);
    va_end(argptr);
    error_message = buffer;
    _what = error_message.c_str();
}

/* See AndorCamera/Error.h */
const char *Error::what() const throw() {
    return _what;
}
