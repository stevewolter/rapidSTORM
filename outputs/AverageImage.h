#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/engine/Image.h>
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
    image::MetaInfo<2>::Resolutions resolution;

    class _Config;
  public:
    typedef simparm::Structure<_Config> Config;
    typedef FileOutputBuilder<AverageImage> Source;

    AverageImage(const Config &config);
    AverageImage *clone() const;
    ~AverageImage();

    AdditionalData announceStormSize(const Announcement &a);
    RunRequirements announce_run(const RunAnnouncement&); 
    void receiveLocalizations(const EngineResult&);
    void store_results();

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
