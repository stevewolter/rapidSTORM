#define cimg_use_magick
#define DSTORM_AVERAGEIMAGE_CPP
#include "outputs/AverageImage.h"
#include "output/Output.h"
#include "output/OutputBuilder.h"
#include "output/FileOutputBuilder.h"
#include "engine/Image.h"
#include "engine/Input.h"
#include "simparm/FileEntry.h"
#include <memory>

#include <iostream>
#include "image/iterator.h"
#include "display/Manager.h"
#include "display/store_image.h"
#include "display/display_normalized.hpp"
#include "image/Image.hpp"
#include "image/extend.h"

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

    void announceStormSize(const Announcement &a) OVERRIDE;
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

    static std::string get_name() { return "AverageImage"; }
    static std::string get_description() { return "Average images"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }

    void attach_ui( simparm::NodeHandle at ) { outputFile.attach_ui( at ); }

};

AverageImage::Config::Config()
: outputFile("ToFile", "Write averaged image to", ".jpg")
{ 
    outputFile.setHelpID( "#AverageImage_ToFile" );
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
        display::store_image(filename, c);
    }
}

AverageImage::RunRequirements AverageImage::announce_run( const RunAnnouncement& )
{
    image.fill(0);
    return RunRequirements();
}

void AverageImage::announceStormSize(const Announcement &a)
{
    boost::shared_ptr<const engine::InputTraits> t = a.input_image_traits;
    if (!a.source_image_is_set || !t) {
        throw std::runtime_error("Input images are not passed to the output "
                + Config::get_description() + ", but are required");
    }
    image = Image(t->image(0).size, 0 * camera::frame);
}

std::auto_ptr< output::OutputSource > make_average_image_source() {
    return std::auto_ptr< output::OutputSource >( new FileOutputBuilder< AverageImage::Config, AverageImage >() );
}

}
}
