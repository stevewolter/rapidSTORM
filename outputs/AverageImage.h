#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Image_impl.h>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <memory>

namespace dStorm {
namespace output {
/** The AverageImage class averages all incoming images into a
  *  single image to show the usefulness of dSTORM. */
class AverageImage : public OutputObject {
  private:
    std::string filename;
    cimg_library::CImg<unsigned long> image;
    ost::Mutex mutex;

    class _Config;
  public:
    typedef simparm::Structure<_Config> Config;
    typedef FileOutputBuilder<AverageImage> Source;

    AverageImage(const Config &config);
    AverageImage *clone() const;

    AdditionalData announceStormSize(const Announcement &a) {
        ost::MutexLock lock(mutex);
        if ( a.carburettor == NULL )
            throw std::logic_error("AverageImage needs access to "
                                   "input driver, but didn't get it.");
        const engine::InputTraits& t = a.carburettor->getTraits();
        image.resize(t.size.x(),t.size.y(), t.size.z(),t.dim);
        image.fill(0);
        return AdditionalData().set_source_image(); 
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
    bool can_work_with(Capabilities cap) 
        { return cap.test( Capabilities::SourceImage ) && 
                 cap.test( Capabilities::InputBuffer ); }
};

}
}
#endif
