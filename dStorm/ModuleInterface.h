#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include <dStorm/BasicOutputs.h>
#include <dStorm/CarConfig.h>

typedef void (*RapidSTORM_Input_Augmenter)
    ( CImgBuffer::Config* inputs );
typedef void (*RapidSTORM_Output_Augmenter)
    ( dStorm::BasicOutputs* outputs );

#endif
