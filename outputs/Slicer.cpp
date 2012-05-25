#include <simparm/BoostUnits.h>
#include <simparm/TabGroup.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <dStorm/UnitEntries/FrameEntry.h>
#include <simparm/Entry.h>
#include <simparm/ChoiceEntry.h>
#include <simparm/ChoiceEntry_Impl.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <stdexcept>

#include "Slicer.h"
#include <sstream>
#include <stdio.h>

#include <dStorm/outputs/NullOutput.h>

namespace dStorm {
namespace slicer {

using namespace output;

class Slicer : public Output {
  public:
    class Config;
  private:
    frame_count slice_size, slice_distance;
    Basename filename;
    simparm::NodeHandle attached_suboutputs;

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
        : suboutputs(c.suboutputs)
        { throw std::logic_error("dStorm::Slicer::Slicer(Copy) undef."); }

    void add_output_clone(int index);

    std::auto_ptr<Announcement> announcement;
    std::auto_ptr<RunAnnouncement> run_announcement;
    void store_results_( bool success );
    void attach_ui_( simparm::NodeHandle at );

  public:
    simparm::TabGroup suboutputs;

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames);

    Slicer( const Config&, std::auto_ptr< output::FilterSource > );
    ~Slicer();

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement& a);
    void receiveLocalizations(const EngineResult&);
};

class Slicer::Config {
  public:
    dStorm::IntFrameEntry slice_size, slice_distance;
    dStorm::output::BasenameAdjustedFileEntry outputFile;

    Config();
    void attach_ui( simparm::NodeHandle at );
};

Basename Slicer::fn_for_slice( int i ) const
{
    Basename bn = filename;
    std::stringstream slice_ident;
    slice_ident << i;
    bn.set_variable("slice", slice_ident.str());
    return bn;
}

void Slicer::add_output_clone(int i) {
    boost::shared_ptr<Output> output;
    try {
        source->FilterSource::set_output_file_basename( 
            fn_for_slice(i) );
        output = source->FilterSource::make_output();
        if ( avoid_filenames != NULL ) 
            output->check_for_duplicate_filenames( *avoid_filenames );
    } catch ( const std::exception& e ) {
        std::cerr << "Building output for slice " << i << " failed: "
                << e.what() << "\n";
        output.reset( new dStorm::outputs::NullOutput() );
    }

    std::stringstream name, desc;
    name << "SlicerNode" << i;
    desc << "Slice " << i;
    boost::shared_ptr<simparm::Object> o 
        ( new simparm::Object(name.str(), desc.str()) );

    output->attach_ui( o->attach_ui( attached_suboutputs ) );
    outputs.replace( i, new Child( output, o ) );

    if ( announcement.get() != NULL )
        outputs[i]->announceStormSize(*announcement);
    if ( run_announcement.get() != NULL )
        outputs[i]->announce_run(*run_announcement);
}

Slicer::Config::Config() 
: slice_size("SliceSize", "Size of one slice in images", 500 * camera::frame),
  slice_distance("SliceDistance", "Start new slice every n images", 100 * camera::frame),
  outputFile("BaseFileName", "File name pattern", "_$slice$")
{
    slice_size.min = 1 * camera::frame;

    slice_distance.min = 1 * camera::frame;

    slice_size.helpID = "#Slicer_Size";
    slice_distance.helpID = "#Slicer_Dist";
    outputFile.helpID = "#Slicer_Pattern";
    outputFile.setHelp("$slice$ is replaced with the block name.");
}

void Slicer::Config::attach_ui( simparm::NodeHandle at )
{
    slice_size.attach_ui( at ); 
    slice_distance.attach_ui( at );
    outputFile.attach_ui( at); 
}

Slicer::Slicer(const Config& config, std::auto_ptr<output::FilterSource> generator )
: slice_size( config.slice_size()  ),
  slice_distance( config.slice_distance() ),
  filename( config.outputFile.get_basename() ),
  source( generator ),
  avoid_filenames(NULL),
  suboutputs( "Outputs", "Outputs to slicer" )
{
    outputs.resize(1, NULL);
    add_output_clone(0);
}

void Slicer::check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
{
    avoid_filenames = &present_filenames;
}

Output::AdditionalData
Slicer::announceStormSize(const Announcement& a)
{
    announcement.reset( new Announcement(a) );

    return outputs[0]->announceStormSize(a);
}

Output::RunRequirements
Slicer::announce_run(const RunAnnouncement& a)
{
    run_announcement.reset( new RunAnnouncement(a) );
    for (unsigned int i = 0; i < outputs.size(); i++)
        if (!outputs.is_null(i))
            outputs[i].images_in_output = 0;

    return outputs[0]->announce_run(a);
}

void Slicer::store_results_( bool success ) {
    for (unsigned int i = 0; i < outputs.size(); i++)
        if (!outputs.is_null(i))
            outputs[i]->store_results( success );
}

void Slicer::receiveLocalizations(const EngineResult& er)
{
    frame_count one_frame( 1 * camera::frame );
    frame_index
        cur_image = er.forImage, 
        back_image = (cur_image >= slice_size) 
                ? (cur_image - (slice_size-one_frame))
                : 0;

    int first_slice = back_image / slice_distance;
    if ( back_image.value() % slice_distance.value()
            != 0 ) 
        first_slice += 1;
    int last_slice = cur_image / slice_distance;
    
    while ( int( outputs.size() ) <= last_slice )
        outputs.push_back( NULL );

    for (int i = first_slice; i <= last_slice; i++) {
        if ( outputs.is_null(i) ) add_output_clone(i);
        outputs[i].images_in_output += one_frame;
        outputs[i]->receiveLocalizations(er);
        if ( outputs[i].images_in_output == slice_size ) {
            outputs[i]->store_results( true );
            outputs.replace(i, NULL);
        }
    }
}

Slicer::~Slicer() 
{
}

void Slicer::attach_ui_( simparm::NodeHandle at ) {
    suboutputs.attach_ui( at );
}

class Source
: public output::FilterSource
{
    Slicer::Config config;
    simparm::TreeObject name_object;
    simparm::Object choice_object;
public:
    Source() 
    : name_object("Slicer", "Slice localization set"),
      choice_object(name_object) 
    {
        choice_object.set_user_level( simparm::Intermediate );
        adjust_to_basename( config.outputFile );
    }

    Source( const Source& o )
    : output::FilterSource(o), config(o.config), 
      name_object(o.name_object),
      choice_object(o.choice_object)
    {
        adjust_to_basename( config.outputFile );
        if ( o.getFactory() != NULL )
            this->set_output_factory( *o.getFactory() );
    }

    Source* clone() const { return new Source(*this); }

    std::auto_ptr<Output> make_output() {
        std::auto_ptr<output::FilterSource> my_clone( clone() );
        return std::auto_ptr<Output>( 
            new Slicer( config, my_clone ) );
    }

    std::string getName() const { return name_object.getName(); }
    std::string getDesc() const { return name_object.getDesc(); }
    void attach_full_ui( simparm::NodeHandle at ) { 
        simparm::NodeHandle r = name_object.attach_ui(at);
        config.attach_ui( r );
        FilterSource::attach_children_ui( r ); 
    }
    void attach_ui( simparm::NodeHandle at ) { choice_object.attach_ui( at ); }
    void hide_in_tree() { name_object.show_in_tree = false; }
};

std::auto_ptr< output::OutputSource > make_output_source() {
    return std::auto_ptr< output::OutputSource >( new Source() );
}

}
}
