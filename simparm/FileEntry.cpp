#include "Entry.h"
#include "FileEntry.h"
#include "Node.h"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

namespace simparm {

NodeHandle FileEntry::create_hidden_node( NodeHandle n ) {
    NodeHandle r = StringEntry::create_hidden_node(n);
    r->add_attribute( default_extension );
    return r;
}

NodeHandle FileEntry::make_naked_node( simparm::NodeHandle node ) {
    return node->create_file_entry( getName() );
}


FileEntry::FileEntry(const FileEntry &entry)
: StringEntry(entry), 
  default_extension(entry.default_extension)
{
}

FileEntry::FileEntry(string name, string desc, string value)
: StringEntry(name, desc, value), 
  default_extension("extension", "")
{
}

FileEntry::FileEntry(string name, string value)
: StringEntry(name, value), 
  default_extension("extension", "")
{
}

FileEntry::~FileEntry() 
{
}

FileEntry &FileEntry::operator=(const string &v)
   { setValue(v); return *this; }

FileEntry::operator bool() const
   { return value() != ""; }


}
