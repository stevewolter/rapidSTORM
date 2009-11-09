#include <dStorm/engine/Image.h>

#include "config.h"
#ifdef HAVE_LIBMAGICK__
#ifndef cimg_use_magick
#define cimg_use_magick
#endif
#endif
#include <CImg.h>
