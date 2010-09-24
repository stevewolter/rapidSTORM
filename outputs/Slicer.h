#ifndef DSTORM_TRANSMISSIONS_SLICER_H
#define DSTORM_TRANSMISSIONS_SLICER_H

#include <dStorm/output/Output.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Structure.hh>
#include <vector>
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
        Output *output;
        Node *node;
        int *usage_counter;
      public:
        frame_count images_in_output;
        ost::Mutex mutex;

        Child() : output(NULL), usage_counter(NULL),
                  images_in_output(0) {}
        Child(const Child& c)
            : output(c.output), node(c.node),
              usage_counter(c.usage_counter),
              images_in_output(c.images_in_output), mutex()
            { if ( usage_counter ) *usage_counter += 1; }
        ~Child() { 
            clear();
        }
        void clear() {
            if ( output != NULL ) {
                *usage_counter -= 1;
                if ( *usage_counter == 0 ) {
                    delete usage_counter;
                    usage_counter = NULL;
                    delete output; 
                    output = NULL;
                    delete node; 
                    node = NULL;
                }
            }
        }
        void set(Output *output, Node *node) {
            clear();
            this->output = output;
            this->node = node;
            usage_counter = new int;
            *usage_counter = 1;
            images_in_output = 0;
        }

        operator bool() const { return output != NULL; }
        Output* operator->() { return output; }
        const Output* operator->() const { return output; }
        Output& operator*() { return *output; }
        const Output& operator*() const { return *output; }
    };
    ost::Mutex outputs_mutex;
    std::vector<Child> outputs;
    std::set<std::string>* avoid_filenames;

    /** Copy constructor undefined. */
    Slicer(const Slicer& c) 
        : OutputObject(c),
          outputs_choice("OutputChoice", "Select slice to display")
        { throw std::logic_error("dStorm::Slicer::Slicer(Copy) undef."); }

    void add_output_clone(int index);

    std::auto_ptr<Announcement> announcement;
    std::auto_ptr<RunAnnouncement> run_announcement;
    std::list<ProgressSignal> received_signals;

  public:

    simparm::NodeChoiceEntry<simparm::Object> outputs_choice;

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames);

    Slicer(const SourceBuilder&);
    Slicer* clone() const;
    ~Slicer();

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement& a);
    void propagate_signal(ProgressSignal);
    Result receiveLocalizations(const EngineResult&);
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
