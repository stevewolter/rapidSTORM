#ifndef DSTORM_RAWIMAGEFILE_H
#define DSTORM_RAWIMAGEFILE_H

#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <tiffio.h>
#include <simparm/FileEntry.hh>

#include <queue>

namespace dStorm {
class RawImageFile : public Output, public simparm::Object {
  private:
    static void error_handler( const char* module,
                               const char* fmt, va_list ap );
    ost::Mutex mutex;

    TIFF *tif;
    tsize_t strip_size;
    tstrip_t strips_per_image;

    unsigned int next_image;
    class LookaheadImg;
    std::priority_queue<LookaheadImg> out_of_time;
    void write_image(const Image& img);

    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::FileOutputBuilder<RawImageFile> Source;

    RawImageFile(const Config&);
    ~RawImageFile();
    RawImageFile* clone() const { 
        throw std::runtime_error(
            "RawImageFile::clone not implemented"); }

    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal);
};

class RawImageFile::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() {
        push_back( outputFile );
    }
  public:
    simparm::FileEntry outputFile;

    _Config();
};

}

#endif
