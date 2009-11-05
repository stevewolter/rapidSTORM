#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include <dStorm/output/BasicOutputs.h>
#include <dStorm/engine/Config.h>
#include <dStorm/input/Config.h>

typedef const char* (*RapidSTORM_Plugin_Desc) ();
typedef void (*RapidSTORM_Input_Augmenter)
    ( CImgBuffer::Config* inputs );
typedef void (*RapidSTORM_Engine_Augmenter)
    ( dStorm::Config* config );
typedef void (*RapidSTORM_Output_Augmenter)
    ( dStorm::BasicOutputs* outputs );

#endif
