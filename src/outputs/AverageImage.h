#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/Output.h>
#include <dStorm/OutputBuilder.h>
#include <dStorm/FileOutputBuilder.h>
#include <dStorm/engine/Image.h>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <memory>
#include <CImg.h>

namespace dStorm {
/** The AverageImage class averages all incoming images into a
  *  single image to show the usefulness of dSTORM. */
class AverageImage : public Output, public simparm::Object {
  private:
    std::string filename;
    cimg_library::CImg<unsigned long> image;
    ost::Mutex mutex;

    class _Config;
  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::FileOutputBuilder<AverageImage> Source;

    AverageImage(const Config &config);
    AverageImage *clone() const;

    AdditionalData announceStormSize(const Announcement &a) {
        ost::MutexLock lock(mutex);
        image.resize(a.width,a.height,a.depth,a.colors);
        image.fill(0);
        return SourceImage; 
    }
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal s);

    const char *getName() { return "AverageImage"; }
};

class AverageImage::_Config : public simparm::Object {
  protected:
    void registerNamedEntries()
        { push_back( outputFile ); }

  public:
    simparm::FileEntry outputFile;
    _Config();
};

}
#endif
