#include "Output.h"
#include "OutputSource.h"
#include <cassert>
#include <sstream>
#include <list>
#include <simparm/FileEntry.hh>
#include <dStorm/input/ImageTraits.h>
#include "doc/help/rapidstorm_help_file.h"

namespace dStorm {

Output::Announcement::Announcement(
    const CImgBuffer::Traits< dStorm::Image >& traits,
    int length,
    Input* carburettor,
    ResultRepeater *repeater)
: traits(traits),
    width(traits.dimx()), height(traits.dimy()), 
    depth(traits.dimz()), colors(traits.dimv()),
    length(length), carburettor(carburettor),
    result_repeater(repeater) {}

class OutputSource::AdjustedList 
: public std::list< simparm::FileEntry* > {};

OutputSource::OutputSource() 
: adjustedList( new AdjustedList() ),
  help_file("help_file", dStorm::HelpFileName)
{
}

OutputSource::OutputSource(const OutputSource& o) 
: simparm::Node(o), simparm::TreeAttributes(o),
  adjustedList( new AdjustedList() ),
  help_file(o.help_file)
{
}

OutputSource::~OutputSource() {
}

void OutputSource::adjust_to_basename(simparm::FileEntry& e)
 
{
    adjustedList->push_back( &e );
}

OutputSource::BasenameResult
OutputSource::set_output_file_basename(
    const std::string& new_basename, std::set<std::string>& avoid)
{
    for ( AdjustedList::iterator i  = adjustedList->begin(); 
                                 i != adjustedList->end(); i++)
    {
        std::string ext = (*i)->default_extension();
        std::string filename = new_basename +ext;
        if ( show_in_tree() ) {
            if ( avoid.insert( filename ).second == true )
                (*i)->value = filename;
        } else {
            if ( avoid.find( filename ) == avoid.end() )
                (*i)->value = filename;
        }
            
    }
    return Basename_Accepted;
}

std::ostream &operator<<(std::ostream &o, 
                         Output::AdditionalData data) {
    std::string rv = "", sep = ", ";
    if ( data & Output::SourceImage )
        rv += sep + "SourceImage";
    if (data & Output::SmoothedImage)
        rv += sep + "SmoothedImage";
    if (data & Output::CandidateTree)
        rv += sep + "CandidateTree";
    if (data & Output::InputBuffer)
        rv += sep + "InputBuffer";
    if (data & Output::LocalizationSources)
        rv += sep + "LocalizationSources";

    rv = rv.substr( sep.size() );
    size_t pos = rv.rfind(',');
    if ( pos != std::string::npos )
        rv.replace( pos, 1, "and" );
    return (o << rv);
}

std::ostream &operator<<(std::ostream &o, 
                         Output::Result r) 
{
    switch( r ) {
      case Output::KeepRunning: 
        return o << "KeepRunning";
      case Output::RemoveThisOutput:
        return o << "RemoveThisOutput";
      case Output::RestartEngine: 
        return o << "RestartEngine";
      case Output::StopEngine: 
        return o << "StopEngine";
      default: return o << "unknown result";
    }
}

std::ostream &operator<<(std::ostream &o, 
                         Output::ProgressSignal p) 
{
    switch( p ) {
      case Output::Engine_run_is_starting:
        return o << "Engine_run_is_starting";
      case Output::Engine_run_is_aborted: 
        return o << "Engine_run_is_aborted";
      case Output::Engine_is_restarted: 
        return o << "Engine_is_restarted";
      case Output::Engine_run_failed: 
        return o << "Engine_run_failed";
      case Output::Engine_run_succeeded: 
        return o << "Engine_run_succeeded";
      case Output::Job_finished_successfully:
        return o << "Job_finished_successfully";
      case Output::Prepare_destruction: 
        return o << "Prepare_destruction";
      default: return o << "unknown signal";
    }
}

void Output::check_additional_data_with_provided
(std::string name, AdditionalData can_provide, AdditionalData are_desired)
{
    if ( ( (~can_provide) & are_desired ) != 0 ) {
        std::stringstream ss;
        ss << "The data " << are_desired << " cannot be provided by "
           << "the data source " << name << ", which can only provide "
           << can_provide;
        throw std::logic_error(ss.str());
    }
}

}
