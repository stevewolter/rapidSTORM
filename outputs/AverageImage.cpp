#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <iostream>
#include <dStorm/Image_iterator.h>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/DisplayDataSource_impl.h>
#include <dStorm/Image_impl.h>

using namespace std;
namespace dStorm {
namespace output {

AverageImage::_Config::_Config()
: Object("AverageImage", "Average images"),
    outputFile("ToFile", "Write averaged image to", ".jpg")
{ 
    outputFile.helpID = "#AverageImage_ToFile";
}

AverageImage::AverageImage( const Config &config )
: OutputObject("AverageImage", "Image averaging status"),
  filename(config.outputFile()) {}

AverageImage* AverageImage::clone() const 
    { return new AverageImage(*this); }

void
AverageImage::receiveLocalizations(const EngineResult& er) {
    engine::Image::const_iterator i = er.source.begin();
    for (Image::iterator j = image.begin(); j != image.end(); j++)
    {
        *j += *i++;
    }
}

void AverageImage::store_results()
{
    if (filename != "" ) {
        Display::Change c(1);
        c.do_clear = true;
        c.clear_image.background = dStorm::Pixel::Black();
        c.display_normalized( image );
        for (int i = 0; i < 2; ++i)
            if ( resolution[i].is_initialized() )
                c.resize_image.pixel_sizes[i] = *resolution[i];
        Display::Manager::getSingleton().store_image(filename, c);
    }
}


}
}
