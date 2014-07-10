#ifndef DSTORM_RAWIMAGEFILE_H
#define DSTORM_RAWIMAGEFILE_H

#include <queue>

#include <tiffio.h>

#include "debug.h"

#include "output/FileOutputBuilder.h"
#include "output/Output.h"
#include "simparm/FileEntry.h"
#include "simparm/ManagedChoiceEntry.h"
#include "simparm/ObjectChoice.h"

namespace dStorm {
namespace output {
class RawImageFile : public Output {
  public:
    enum OutputType {
        Signal,
        BackgroundCorrectedSignal,
        Background,
    };

  private:
    simparm::NodeHandle current_ui;
    static void error_handler( const char* module,
                               const char* fmt, va_list ap );

    std::string filename;
    OutputType output_type;

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
    class OutputTypeChoice;

    RawImageFile(const Config&);
    ~RawImageFile();
    RawImageFile* clone() const { 
        throw std::runtime_error(
            "RawImageFile::clone not implemented"); }

    void announceStormSize(const Announcement &a) OVERRIDE;
    void receiveLocalizations(const EngineResult&) OVERRIDE;

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }
};

class RawImageFile::OutputTypeChoice : public simparm::ObjectChoice {
  public:
    const OutputType output_type;
    OutputTypeChoice(std::string name, std::string desc, OutputType output_type)
        : simparm::ObjectChoice(name, desc), output_type(output_type) {}

    void attach_ui( simparm::NodeHandle to ) { attach_parent(to); }
    OutputTypeChoice* clone() const { return new OutputTypeChoice(*this); }
};

class RawImageFile::Config {
  public:
    BasenameAdjustedFileEntry outputFile;
    simparm::ManagedChoiceEntry<OutputTypeChoice> save_background;

    Config();
    static std::string get_name() { return "RawImage"; }
    static std::string get_description() { return "Save raw images"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }
    void attach_ui( simparm::NodeHandle at ) {
        outputFile.attach_ui( at );
        save_background.attach_ui( at );
    }
};

}
}

#endif
