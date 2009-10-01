#include "transmissions/Slicer.h"
#include <sstream>
#include "help_context.h"

#include <transmissions/Viewer.h>

namespace dStorm {

const char *format_string(const std::string& filename, int block)
{
    static char formatted[1024];
    snprintf( formatted, 1024, filename.c_str(), block );
    return formatted;
}
    
void Slicer::add_output_clone(int i) {
    /* To test for exceptions on output construction. */
    const char *output_basename = format_string( filename, i);
    if ( filename != "" )
        this->source->set_forward_output_file_basename( output_basename  );
    std::stringstream name, desc;
    name << "SlicerNode" << i;
    desc << "Slice " << i;
    simparm::Object* o = new simparm::Object(name.str(), desc.str());

    outputs[i].set( this->source->make_forward_output().release(), o );
    o->push_back( *outputs[i] );
    outputs_choice.push_back( *o );

    if ( announcement.get() != NULL )
        outputs[i]->announceStormSize(*announcement);
}

Slicer::_Config::_Config() 
: simparm::Object("Slicer", "Slice localization set"),
  slice_size("SliceSize", "Size of one slice in images"),
  slice_distance("SliceDistance", "Start new slice every n images"),
  filename("BaseFileName", "File name pattern"),
  avoid_filenames(NULL)
{
    slice_size.helpID = HELP_Slicer_Size;
    slice_distance.helpID = HELP_Slicer_Dist;
    filename.helpID = HELP_Slicer_Pattern;
    filename.setHelp("%i is replaced with the block name.");
}

Slicer::Slicer(const Source& source)
 
: simparm::Object("Slicer", "Object Slicer"),
  slice_size( source.slice_size() ),
  slice_distance( source.slice_distance() ),
  filename( source.filename() ),
  source( source.clone() ),
  outputs_choice( "Outputs", "Outputs to slicer" )
{
    outputs.resize(1);
    add_output_clone(0);
    push_back( outputs_choice );
}

Output::AdditionalData
Slicer::announceStormSize(const Announcement& a)
{
    announcement.reset( new Announcement(a) );
    unsigned int number_of_slices = a.length / slice_distance;
    if ( a.length % slice_distance > 0 ) number_of_slices++;

    outputs.resize( number_of_slices );

    return outputs[0]->announceStormSize(a);
}

void Slicer::propagate_signal(ProgressSignal s) {
    received_signals.push_back(s);
    for (unsigned int i = 0; i < outputs.size(); i++)
        if (outputs[i]) {
            if ( s == Engine_is_restarted )
                outputs[i].images_in_output = 0;
            outputs[i]->propagate_signal(s);
        }
}

Output::Result Slicer::receiveLocalizations(const EngineResult& er)
{
    unsigned int 
        cur_image = er.forImage, 
        back_image = (cur_image >= slice_size) 
                        ? (cur_image - (slice_size-1))
                        : 0;

    int first_slice = ( back_image % slice_distance == 0 )
            ? back_image / slice_distance
            : (back_image / slice_distance) + 1,
        last_slice = cur_image / slice_distance;
    
    for (int i = first_slice; i <= last_slice; i++) {
        ost::MutexLock lock( outputs[i].mutex );
        if ( !outputs[i] ) add_output_clone(i);
        outputs[i].images_in_output++;
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
