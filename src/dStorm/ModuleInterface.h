#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include <dStorm/BasicOutputs.h>
#include <CImgBuffer/Config.h>

typedef const char* (*RapidSTORM_Plugin_Desc) ();
typedef void (*RapidSTORM_Input_Augmenter)
    ( CImgBuffer::Config* inputs );
typedef void (*RapidSTORM_Output_Augmenter)
    ( dStorm::BasicOutputs* outputs );

#endif
