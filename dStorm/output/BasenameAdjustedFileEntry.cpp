#include "BasenameAdjustedFileEntry.h"

namespace dStorm {
namespace output {

BasenameAdjustedFileEntry:: BasenameAdjustedFileEntry
( 
    std::string name,
    std::string desc,
    std::string suffix
) 
: simparm::FileEntry(name,desc),
  has_been_user_modified(false)
{
    registerNamedEntries();
    default_extension = suffix;
}

BasenameAdjustedFileEntry::BasenameAdjustedFileEntry
    (const BasenameAdjustedFileEntry& o)
: simparm::FileEntry(o),
  simparm::Listener( simparm::Event::ValueChanged ),
  last_basename(o.last_basename),
  has_been_user_modified(o.has_been_user_modified),
  expect_change(o.expect_change)
{
    registerNamedEntries();
}

void BasenameAdjustedFileEntry::
    registerNamedEntries()
{
    receive_changes_from( value );
}

void BasenameAdjustedFileEntry::
    operator()( const simparm::Event& e )
{
    if ( expect_change ) {
        return;
    } else {
        std::string norm = last_basename.new_basename() + default_extension();
        has_been_user_modified = ( value() == norm );
    } 
}

void BasenameAdjustedFileEntry::
set_output_file_basename(
    const Basename& basename 
)
{
    if ( ! has_been_user_modified ) {
        expect_change = true;
        value = basename.unformatted()()
                  + default_extension();
        expect_change = false;
    }
    last_basename = basename;
}

std::string BasenameAdjustedFileEntry::operator()() const
{ 
    return last_basename.new_basename() 
           + default_extension(); 
}

}
}
