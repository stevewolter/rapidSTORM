#include "Slicer.h"
#include <sstream>
#include "doc/help/context.h"
#include <stdio.h>

#include <dStorm/outputs/NullOutput.h>

namespace dStorm {
namespace output {

const char *format_string(const std::string& filename, int block)
{
    static char formatted[1024];
    snprintf( formatted, 1024, filename.c_str(), block );
    return formatted;
}
    
void Slicer::add_output_clone(int i) {
    std::auto_ptr<Output> output;
    try {
        source->FilterSource::set_output_file_basename( 
            format_string( filename.c_str(), i ) );
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
    simparm::Object* o = new simparm::Object(name.str(), desc.str());

    outputs[i].set( output.release(), o );
    o->push_back( *outputs[i] );
    outputs_choice.push_back( *o );

    if ( announcement.get() != NULL )
        outputs[i]->announceStormSize(*announcement);
}

Slicer::_Config::_Config() 
: simparm::Object("Slicer", "Slice localization set"),
  slice_size("SliceSize", "Size of one slice in images"),
  slice_distance("SliceDistance", "Start new slice every n images"),
  filename("BaseFileName", "File name pattern")
{
    slice_size.helpID = HELP_Slicer_Size;
    slice_size.min = 1;

    slice_distance.helpID = HELP_Slicer_Dist;
    slice_distance.min = 1;

    filename.helpID = HELP_Slicer_Pattern;
    filename.setHelp("%i is replaced with the block name.");
}

Slicer::Slicer(const Source& source)
 
: OutputObject("Slicer", "Object Slicer"),
  slice_size( source.slice_size() * cs_units::camera::frame ),
  slice_distance( source.slice_distance() * cs_units::camera::frame),
  filename( source.filename() ),
  source( source.clone() ),
  avoid_filenames(NULL),
  outputs_choice( "Outputs", "Outputs to slicer" )
{
    outputs.resize(1);
    add_output_clone(0);
    push_back( outputs_choice );
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

void Slicer::propagate_signal(ProgressSignal s) {
    received_signals.push_back(s);
    ost::MutexLock lock( outputs_mutex );
    for (unsigned int i = 0; i < outputs.size(); i++)
        if (outputs[i]) {
            if ( s == Engine_is_restarted )
                outputs[i].images_in_output = 0;
            outputs[i]->propagate_signal(s);
        }
}

Output::Result Slicer::receiveLocalizations(const EngineResult& er)
{
    frame_count one_frame( 1 * cs_units::camera::frame );
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
    
    if ( int( outputs.size() ) <= last_slice ) {
        ost::MutexLock lock( outputs_mutex );
        while ( int( outputs.size() ) <= last_slice )
            outputs.push_back( Child() );
    }

    for (int i = first_slice; i <= last_slice; i++) {
        ost::MutexLock lock( outputs[i].mutex );
        if ( !outputs[i] ) add_output_clone(i);
        outputs[i].images_in_output += one_frame;
        outputs[i]->receiveLocalizations(er);
        if ( outputs[i].images_in_output == slice_size ) {
            outputs[i]->propagate_signal( Engine_run_succeeded );
            outputs[i]->propagate_signal( Job_finished_successfully );
            outputs[i]->propagate_signal( Prepare_destruction );
            outputs[i].clear();
        }
    }

    return KeepRunning;
}

Slicer::~Slicer() 
{
}

}
}
