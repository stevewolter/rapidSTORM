
// SimParm: Simple and flexible C++ configuration framework
// Copyright (C) 2007 Australian National University
// 
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
// 
// Contact:
// Kevin Pulo
// kevin.pulo@anu.edu.au
// Leonard Huxley Bldg 56
// Australian National University, ACT, 0200, Australia

#include "Entry.hh"
#include "FileEntry.hh"
#include <stdlib.h>
#include <sstream>
#include <fstream>

namespace simparm {

Entry::Entry(string name, string desc)
: Object(name, desc),
  help("help", ""),
  invalid("invalid", false),
  editable("editable", true),
  outputOnChange("outputOnChange", true),
  helpID("helpID", 0)
{
    push_back(help);
    push_back(invalid);
    push_back(editable);
    push_back(outputOnChange);
    push_back(helpID);
}

Entry::Entry(const Entry& from)
:   Node(from),
    Object(from),
    help(from.help),
    invalid(from.invalid),
    editable(from.editable),
    outputOnChange(from.outputOnChange),
    helpID(from.helpID)
{
    push_back(help);
    push_back(invalid);
    push_back(editable);
    push_back(outputOnChange);
    push_back(helpID);
}

Entry::~Entry() 
{}

void formatParagraph(ostream &o, unsigned int left_col, 
                   unsigned int right_col, const string &s) 
{
   unsigned int pos, lookahead = 0;
   unsigned int cur_col = left_col;
   while (lookahead < s.length()) {
      pos = lookahead;
      if (isalpha(s[lookahead]))
         while (lookahead < s.length() && 
                isalpha(s[lookahead])) lookahead++;
      else
         lookahead++;

      if ((lookahead-pos) > 1+(right_col - cur_col)) {
         o << "\n";
         cur_col = 0;
         while (cur_col < left_col) { cur_col++; o << " "; }
      }
      if (cur_col == left_col && isspace(s[pos]))
         /* skip */;
      else
         o << s.substr(pos, lookahead-pos);
      cur_col += lookahead - pos;
   }
   while (cur_col++ <= right_col) o << " ";
}

void Entry::printHelp(ostream &o) const {
   string n = "--" + name().substr(0, std::min<int>(name().length(), 19));
   formatParagraph(o, 0, 20, n);
   o << "  ";
   formatParagraph(o, 23, 79, desc());
   o << "\n";
   if (help() != "") {
      for (int i = 0; i < 23; i++) o << " ";
      formatParagraph(o, 23, 79, help());
      o << "\n";
   }
}

FileEntry::FileEntry(const FileEntry &entry)
: Node(entry),
  StringEntry(entry), out_stream(NULL), in_stream(NULL),
  default_extension(entry.default_extension)
{
    push_back( default_extension );
}
FileEntry::FileEntry(string name, string desc, string value)
: Node(),
  StringEntry(name, desc, value), out_stream(NULL), in_stream(NULL),
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

template <> const char *typeName<bool>() { return "Bool"; }
template <> const char *typeName<char>() { return "Char"; }
template <> const char *typeName<unsigned char>() 
    { return "UnsignedChar"; }
template <> const char *typeName<short>() { return "Short"; }
template <> const char *typeName<unsigned short>() 
    { return "UnsignedShort"; }
template <> const char *typeName<int>() { return "Int"; }
template <> const char *typeName<unsigned int>() 
    { return "UnsignedInt"; }
template <> const char *typeName<long>() { return "Long"; }
template <> const char *typeName<unsigned long>() 
    { return "UnsignedLong"; }
template <> const char *typeName<long long>() { return "LongLong"; }
template <> const char *typeName<unsigned long long>() 
    { return "UnsignedLongLong"; }
template <> const char *typeName<float>() { return "Float"; }
template <> const char *typeName<double>() { return "Double"; }
template <> const char *typeName<long double>() 
    { return "LongDouble"; }
template <> const char *typeName<string>() { return "String"; }

}

#include "Entry_Impl.hh"
namespace simparm {
    template class EntryType<string>;
    template class EntryType<bool>;
    template class EntryType<unsigned long>;
};

#include "NumericEntry_Impl.hh"

namespace simparm {
    template class NumericEntry<short int>;
    template class NumericEntry<int>;
    template class NumericEntry<long int>;
    template class NumericEntry<unsigned short int>;
    template class NumericEntry<unsigned int>;
    template class NumericEntry<unsigned long int>;
    template class NumericEntry<float>;
    template class NumericEntry<double>;
    template class NumericEntry<long double>;
};
