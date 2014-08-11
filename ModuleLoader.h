#ifndef DSTORM_MODULELOADER_H
#define DSTORM_MODULELOADER_H

#include <memory>

#include "input/Link.h"
#include "output/LocalizedImage.h"

namespace dStorm {

class Config;

void add_output_modules( Config& );

std::unique_ptr<input::Link<output::LocalizedImage>> create_image_input();
std::unique_ptr<input::Link<output::LocalizedImage>> create_localizations_input();

}

#endif
