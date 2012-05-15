#include "Entry.hh"
#include "FileEntry.hh"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string.h>

namespace simparm {

FileEntry::FileEntry(const FileEntry &entry)
: StringEntry(entry), out_stream(NULL), in_stream(NULL),
  default_extension(entry.default_extension)
{
    push_back( default_extension );
}
FileEntry::FileEntry(string name, string desc, string value)
: StringEntry(name, desc, value), out_stream(NULL), in_stream(NULL),
  default_extension("extension", "")
{
    push_back( default_extension );
}
FileEntry::~FileEntry() 
{
    close_output_stream();
    close_input_stream();
}

FileEntry &FileEntry::operator=(const string &v)
   { setValue(v); return *this; }

FileEntry::operator bool() const
   { return value() != ""; }

std::istream& FileEntry::get_input_stream() {
    if ( bool(*this) ) {
        if (in_stream != NULL) {
            return *in_stream;
        } else {
            if (value() == "-")
                in_stream = &std::cin;
            else {
                string filename = value();
                openedIStream.reset(
                    new std::ifstream(filename.c_str(), std::ios::in));
                in_stream = openedIStream.get();
            }
            return *in_stream;
        }
    } else 
        throw std::logic_error("Tried to open stream for FileEntry "
                               "that had no filename set.");
}

std::ostream& FileEntry::get_output_stream(bool append)
 
{
    if ( bool(*this) ) {
        if (out_stream != NULL) {
            return *out_stream;
        } else {
            if (value() == "-")
                out_stream = &std::cout;
            else {
                string filename = value();
                openedOStream.reset(
                    new std::ofstream(filename.c_str(), 
                    std::ios::out | ((append) ? std::ios::app : std::ios::out)));
                out_stream = openedOStream.get();
            }
            return *out_stream;
        }
    } else 
        throw std::logic_error("Tried to open stream for FileEntry "
                               "that had no filename set.");
}

void FileEntry::close_output_stream() {
    if (out_stream != NULL) { 
        openedOStream.reset(NULL);
        out_stream = NULL; 
    }
}
void FileEntry::close_input_stream() {
    if (in_stream != NULL) { 
        openedIStream.reset(NULL);
        in_stream = NULL; 
    }
}

std::string FileEntry::without_extension() const {
    typedef simparm::Attribute<std::string> StrAttr;

    /* Extension attributes start with this name. */
    const std::string def_ext = "extension";

    std::string rv = value();
    /* Find all string attribute childrens of watch. */
    for (simparm::Node::const_iterator i=begin(); i!=end(); i++)
    {
        const StrAttr *a;
        std::string name = i->getName();
        if ( (a = dynamic_cast<const StrAttr*>(&*i)) != NULL &&
                name.substr(0,def_ext.size()) == def_ext )
        {
            /* This is an attribute for a filename extension.
                * Try to find this extension on the file. */
            std::string extension = (*a)();
            int ext_size = extension.size();
            if ( ext_size == 0 || int(rv.size()) < ext_size ) 
                continue;

            std::string file_extension_part = 
                rv.substr( rv.size() - ext_size, ext_size );

            if ( 0 == strcasecmp( file_extension_part.c_str(),
                                    extension.c_str() ) )
                return rv.substr( 0, rv.size() - ext_size );
        }
    }
    return value();
}

}
