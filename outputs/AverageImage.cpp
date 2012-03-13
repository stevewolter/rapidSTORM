#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <iostream>
#include <dStorm/Image_iterator.h>
#include <dStorm/display/Manager.h>
#include <dStorm/display/display_normalized.hpp>
#include <dStorm/Image_impl.h>
#include <dStorm/image/extend.h>

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

AverageImage::~AverageImage() {}

void
AverageImage::receiveLocalizations(const EngineResult& er) {
    engine::Image2D::const_iterator i = er.source->plane(0).begin();
    for (Image::iterator j = image.begin(); j != image.end(); j++)
    {
        *j += *i++;
    }
}

void AverageImage::store_results_( bool )
{
    if (filename != "" ) {
        display::Change c(1);
        c.do_clear = true;
        c.clear_image.background = dStorm::Pixel::Black();
        display_normalized( c, extend( image, dStorm::Image<unsigned long,display::Image::Dim>() ) );
        for (int i = 0; i < 2; ++i)
            if ( resolution[i].is_initialized() )
                c.resize_image.pixel_sizes[i] = *resolution[i];
        display::Manager::getSingleton().store_image(filename, c);
    }
}

AverageImage::RunRequirements AverageImage::announce_run( const RunAnnouncement& )
{
    image.fill(0);
    return RunRequirements();
}

AverageImage::AdditionalData AverageImage::announceStormSize(const Announcement &a)
{
    boost::shared_ptr<const engine::InputTraits> t = a.input_image_traits;
    image = Image(t->image(0).size, 0 * camera::frame);
    return AdditionalData().set_source_image(); 
}

}
}
