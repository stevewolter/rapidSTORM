#include <dStorm/ModuleInterface.h>
#include "NoiseSource.h"

#ifdef __cplusplus
extern "C" {
#endif

void rapidSTORM_Input_Augmenter ( CImgBuffer::Config* inputs ) {
    inputs->inputMethod.addChoice(
        new locprec::NoiseConfig<unsigned short>( *inputs ));
}

void (*rapidSTORM_Output_Augmenter)
    ( dStorm::BasicOutputs* outputs )
{
}


#ifdef __cplusplus
}
#endif
