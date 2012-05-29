#ifndef DSTORM_RAWIMAGEFILE_H
#define DSTORM_RAWIMAGEFILE_H

#include "debug.h"

#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <tiffio.h>
#include <simparm/FileEntry.h>

#include <queue>

namespace dStorm {
namespace output {
class RawImageFile : public Output {
  private:
    simparm::NodeHandle current_ui;
    static void error_handler( const char* module,
                               const char* fmt, va_list ap );

    std::string filename;

    TIFF *tif;
    tsize_t strip_size;
    tstrip_t strips_per_image;

    frame_count next_image;
    void write_image(const engine::ImageStack& img);

    std::vector< image::MetaInfo<2> > size;
    boost::optional<frame_count> last_frame;
    void store_results_( bool );
    void attach_ui_( simparm::NodeHandle n ) { current_ui = n; }

  public:
    class Config;

    RawImageFile(const Config&);
    ~RawImageFile();
    RawImageFile* clone() const { 
        throw std::runtime_error(
            "RawImageFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    void receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }
};

class RawImageFile::Config {
  public:
    BasenameAdjustedFileEntry outputFile;

    Config();
    static std::string get_name() { return "RawImage"; }
    static std::string get_description() { return "Save raw images"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }
    bool can_work_with(Capabilities cap) { 
        return cap.test( Capabilities::SourceImage ); 
    }
    void attach_ui( simparm::NodeHandle at ) {
        outputFile.attach_ui( at );
    }
};

}
}

#endif
