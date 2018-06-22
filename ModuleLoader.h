#ifndef DSTORM_MODULELOADER_H
#define DSTORM_MODULELOADER_H

namespace dStorm {

class Config;

void add_output_modules( Config& );
void add_image_input_modules( Config& );
void add_stm_input_modules( Config& );

}

#endif
