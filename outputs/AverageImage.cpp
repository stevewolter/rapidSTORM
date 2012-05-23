#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "AverageImage.h"
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Input.h>
#include <simparm/FileEntry.hh>
#include <memory>

#include <iostream>
#include <dStorm/Image_iterator.h>
#include <dStorm/display/Manager.h>
#include <dStorm/display/display_normalized.hpp>
#include <dStorm/Image_impl.h>
#include <dStorm/image/extend.h>

using namespace std;
namespace dStorm {
namespace output {

/** The AverageImage class averages all incoming images into a
  *  single image to show the usefulness of dSTORM. */
class AverageImage : public Output {
  private:
    std::string filename;
    typedef dStorm::Image<unsigned long,2> Image;
    Image image;
    image::MetaInfo<2>::Resolutions resolution;

    void store_results_( bool success );
  public:
    class Config;

    AverageImage(const Config &config);
    ~AverageImage();

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&); 
    void receiveLocalizations(const EngineResult&);

    const char *getName() { return "AverageImage"; }

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }

};

class AverageImage::Config {
  public:
    BasenameAdjustedFileEntry outputFile;
    Config();
    bool can_work_with(Capabilities cap) 
        { return cap.test( Capabilities::SourceImage ) && 
                 cap.test( Capabilities::InputBuffer ); }

    static std::string get_name() { return "AverageImage"; }
    static std::string get_description() { return "Average images"; }

    void attach_ui( simparm::Node& at ) { outputFile.attach_ui( at ); }

};

AverageImage::Config::Config()
: outputFile("ToFile", "Write averaged image to", ".jpg")
{ 
    outputFile.helpID = "#AverageImage_ToFile";
}

AverageImage::AverageImage( const Config &config )
: filename(config.outputFile()) {}

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

std::auto_ptr< output::OutputSource > make_average_image_source() {
    return std::auto_ptr< output::OutputSource >( new FileOutputBuilder< AverageImage::Config, AverageImage >() );
}

}
}
