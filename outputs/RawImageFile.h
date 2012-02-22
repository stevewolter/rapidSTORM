#ifndef DSTORM_RAWIMAGEFILE_H
#define DSTORM_RAWIMAGEFILE_H

#include "debug.h"

#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <tiffio.h>
#include <simparm/FileEntry.hh>

#include <queue>

namespace dStorm {
namespace output {
class RawImageFile : public OutputObject {
  private:
    static void error_handler( const char* module,
                               const char* fmt, va_list ap );

    std::string filename;

    TIFF *tif;
    tsize_t strip_size;
    tstrip_t strips_per_image;

    frame_count next_image;
    void write_image(const engine::ImageStack& img);

    class _Config;

    std::vector< image::MetaInfo<2> > size;
    boost::optional<frame_count> last_frame;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef FileOutputBuilder<RawImageFile> Source;

    RawImageFile(const Config&);
    ~RawImageFile();
    RawImageFile* clone() const { 
        throw std::runtime_error(
            "RawImageFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    void receiveLocalizations(const EngineResult&);
    void store_results();

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }
};

class RawImageFile::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
    }
  public:
    BasenameAdjustedFileEntry outputFile;

    _Config();
    bool can_work_with(Capabilities cap) { 
        return cap.test( Capabilities::SourceImage ); 
    }
};

}
}

#endif
