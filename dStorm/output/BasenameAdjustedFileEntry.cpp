#include "BasenameAdjustedFileEntry.h"
#include "debug.h"

namespace dStorm {
namespace output {

BasenameAdjustedFileEntry:: BasenameAdjustedFileEntry
( 
    std::string name,
    std::string desc,
    std::string suffix
) 
: simparm::FileEntry(name,desc, ""),
  has_been_user_modified(false)
{
    default_extension = suffix;
}

void BasenameAdjustedFileEntry::attach_ui( simparm::NodeHandle to )
{
    simparm::FileEntry::attach_ui( to );
    listening = value.notify_on_value_change
        ( boost::bind( &BasenameAdjustedFileEntry::file_name_changed, this ) );
}

void BasenameAdjustedFileEntry::file_name_changed()
{
    if ( expect_change ) {
        DEBUG("Doing nothing because change was expected");
        return;
    } else {
        std::string norm = last_basename.unformatted()() + default_extension();
        if ( value () == "" ) {
            has_been_user_modified = false;
            value = norm;
        } else {
            has_been_user_modified = ( value() != norm );
        }
        DEBUG("After change set has_been_user_modified to " << has_been_user_modified << ", new value is " << value());
    } 
}

void BasenameAdjustedFileEntry::
set_output_file_basename(
    const Basename& basename 
)
{
    if ( ! has_been_user_modified ) {
        DEBUG("Entry has not been user-modified, setting value");
        expect_change = true;
        value = basename.unformatted()() + default_extension();
        expect_change = false;
        DEBUG("Set value to " << value());
    } else {
        DEBUG("Entry has been user-modified, not setting basename " << basename.unformatted()());
    }
    last_basename = basename;
}

std::string BasenameAdjustedFileEntry::operator()() const
{ 
    Basename b = last_basename;
    b.unformatted() = value();
    return b.new_basename();
}

Basename BasenameAdjustedFileEntry::get_basename() const
{
    Basename b = last_basename;
    b.unformatted() = value();
    return b;
}

}
}
