#ifndef DSTORM_TRANSMISSIONS_SLICER_H
#define DSTORM_TRANSMISSIONS_SLICER_H

#include <simparm/BoostUnits.hh>
#include <dStorm/output/Output.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Structure.hh>
#include <boost/ptr_container/ptr_vector.hpp>
#include <stdexcept>

namespace dStorm {
namespace output {
class Slicer : public OutputObject {
  public:
    class _Config;
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::output::OutputBuilder<Slicer,FilterSource>
        SourceBuilder;
    typedef dStorm::output::OutputFileAdjuster<SourceBuilder> Source;
  private:
    frame_count slice_size, slice_distance;
    Basename filename;

    Basename fn_for_slice( int i ) const;

    std::auto_ptr<dStorm::output::FilterSource> source;

    class Child {
        boost::shared_ptr<simparm::Object> node;
        boost::shared_ptr<Output> output;
      public:
        frame_count images_in_output;

        Child(boost::shared_ptr<Output> output, 
             boost::shared_ptr<simparm::Object> node) 
            : node(node), output(output), images_in_output(0) {}
        ~Child() {}

        operator bool() const { return output != NULL; }
        Output* operator->() { return output.get(); }
        const Output* operator->() const { return output.get(); }
        Output& operator*() { return *output; }
        const Output& operator*() const { return *output; }
    };
    boost::ptr_vector< boost::nullable<Child> > outputs;
    std::set<std::string>* avoid_filenames;

    /** Copy constructor undefined. */
    Slicer(const Slicer& c) 
        : OutputObject(c),
          outputs_choice("OutputChoice", "Select slice to display")
        { throw std::logic_error("dStorm::Slicer::Slicer(Copy) undef."); }

    void add_output_clone(int index);

    std::auto_ptr<Announcement> announcement;
    std::auto_ptr<RunAnnouncement> run_announcement;
    void store_results_( bool success );

  public:

    simparm::NodeChoiceEntry<simparm::Object> outputs_choice;

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames);

    Slicer(const SourceBuilder&);
    Slicer* clone() const;
    ~Slicer();

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement& a);
    void receiveLocalizations(const EngineResult&);
};

class Slicer::_Config : public simparm::Object {
  protected:
    void registerNamedEntries();
  public:
    dStorm::IntFrameEntry slice_size, slice_distance;
    dStorm::output::BasenameAdjustedFileEntry outputFile;

    _Config();

    bool can_work_with( Capabilities& cap ) 
        { return true; }
};


}
}
#endif
