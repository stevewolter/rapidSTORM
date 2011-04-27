#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/engine/Input.h>
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
    typedef dStorm::Image<unsigned long,2> Image;
    Image image;
    boost::array< input::ImageResolution, 2 > resolution;
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
        boost::shared_ptr<engine::InputTraits> t = a.carburettor->get_traits();
        image = Image(t->size.start<2>(), 0 * camera::frame);
        image.fill(0);
        return AdditionalData().set_source_image(); 
    }
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal s);

    const char *getName() { return "AverageImage"; }

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
        { insert_filename_with_check( filename, present_filenames ); }

};

class AverageImage::_Config : public simparm::Object {
  protected:
    void registerNamedEntries()
        { push_back( outputFile ); }

  public:
    BasenameAdjustedFileEntry outputFile;
    _Config();
    bool can_work_with(Capabilities cap) 
        { return cap.test( Capabilities::SourceImage ) && 
                 cap.test( Capabilities::InputBuffer ); }
};

}
}
#endif
