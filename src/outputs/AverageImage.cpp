#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <iostream>
#include "doc/help/context.h"
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
    userLevel = Intermediate;
    outputFile.helpID = HELP_AverageImage_ToFile;
}

AverageImage::AverageImage( const Config &config )
: OutputObject("AverageImage", "Image averaging status"),
  filename(config.outputFile()) {}

AverageImage* AverageImage::clone() const 
    { return new AverageImage(*this); }

Output::Result
AverageImage::receiveLocalizations(const EngineResult& er) {
    ost::MutexLock lock(mutex);
    engine::Image::const_iterator i = er.source->begin();
    for (Image::iterator j = image.begin(); j != image.end(); j++)
    {
        *j += *i++;
    }

    return KeepRunning;
}

void AverageImage::propagate_signal(Output::ProgressSignal s)
 
{
    if (s == Engine_run_succeeded && filename != "" ) {
        ost::MutexLock lock(mutex);
        Display::Change c(1);
        c.do_clear = true;
        c.clear_image.background = dStorm::Pixel::Black();
        c.display_normalized( image );
        c.resize_image.pixel_size = *resolution;
        Display::Manager::getSingleton().store_image(filename, c);
    } else if (s == Engine_run_failed) {
        ost::MutexLock lock(mutex);
        image.fill(0);
    }
}


}
}
