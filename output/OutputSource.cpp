#include "output/Output.h"
#include "output/OutputSource.h"
#include "output/BasenameAdjustedFileEntry.h"
#include <cassert>
#include <sstream>
#include <list>
#include "simparm/FileEntry.h"

namespace dStorm {
namespace output {

class OutputSource::AdjustedList 
: public std::list< BasenameAdjustedFileEntry* > {};

OutputSource::OutputSource() 
: destruction("RemoveOutput", "Remove output"),
  adjustedList( new AdjustedList() )
{
}

OutputSource::OutputSource(const OutputSource& o) 
: destruction(o.destruction),
  adjustedList( new AdjustedList() )
{
}

OutputSource::~OutputSource() {
}

void OutputSource::adjust_to_basename(BasenameAdjustedFileEntry& e)
{
    adjustedList->push_back( &e );
}

void
OutputSource::set_output_file_basename(const Basename& new_basename)
{
    for ( AdjustedList::iterator i  = adjustedList->begin(); 
                                 i != adjustedList->end(); i++)
    {
        (*i)->set_output_file_basename(
            new_basename );
    }
}

void Output::insert_filename_with_check(
    std::string file, std::set<std::string>& present_filenames )
{
    if ( file != "" && present_filenames.insert( file ).second == false )
        throw std::runtime_error(
            "The output file name '" + file + "' was used by multiple "
            "outputs. Refusing to start job to avoid conflict.");
}

void OutputSource::destruction_clicked() {
    if ( destruction.triggered() ) {
        destruction.untrigger();
        destruction_desired();
    }
}

void OutputSource::attach_destruction_trigger( simparm::NodeHandle at ) {
    listen = destruction.value.notify_on_value_change( boost::bind( &OutputSource::destruction_clicked, this ) );
    destruction.attach_ui(at);
}

boost::signals2::connection OutputSource::notify_when_destruction_is_desired(
    boost::signals2::slot<void()> callback )
{
    return destruction_desired.connect( callback );
}

}
}
