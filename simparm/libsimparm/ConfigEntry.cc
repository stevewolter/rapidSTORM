
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

#include "ConfigEntry.hh"
#include <stdlib.h>
#include <sstream>
#include <fstream>

namespace simparm {

ConfigEntry::ConfigEntry(string name, string desc) throw()
: ConfigObject(name, desc),
  help("help", ""),
  invalid("invalid", false),
  viewable("viewable", true),
  editable("editable", true),
  outputOnChange("outputOnChange", true),
  userLevel("userLevel", Beginner)
{
    push_back(help);
    push_back(invalid);
    push_back(viewable);
    push_back(editable);
    push_back(outputOnChange);
    push_back(userLevel);
}

ConfigEntry::ConfigEntry(const ConfigEntry& from) throw()
:   ConfigObject(from),
    help(from.help),
    invalid(from.invalid),
    viewable(from.viewable),
    editable(from.editable),
    outputOnChange(from.outputOnChange),
    userLevel(from.userLevel)
{
    push_back(help);
    push_back(invalid);
    push_back(viewable);
    push_back(editable);
    push_back(outputOnChange);
    push_back(userLevel);
}

ConfigEntry::~ConfigEntry() throw()
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

void ConfigEntry::printHelp(ostream &o) const throw() {
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

ConfigEntryFile::ConfigEntryFile(const ConfigEntryFile &entry)
: ConfigObject(entry),
  ConfigEntryString(entry), out_stream(NULL), in_stream(NULL)
{
}
ConfigEntryFile::ConfigEntryFile(string name, string desc, string value)
: ConfigObject(name, desc),
  ConfigEntryString(name, desc, value), out_stream(NULL), in_stream(NULL)
{}
ConfigEntryFile::~ConfigEntryFile() throw() {}

const char *ConfigEntryFile::get_type() const
   { return "ConfigEntryFile"; }
ConfigEntryFile &ConfigEntryFile::operator=(const string &v)
   { setValue(v); return *this; }

ConfigEntryFile::operator bool() const
   { return value() != ""; }

std::istream& ConfigEntryFile::getInputStream() throw(std::exception) {
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
        throw std::logic_error("Tried to open stream for ConfigEntryFile "
                               "that had no filename set.");
}

std::ostream& ConfigEntryFile::getOutputStream(bool append)
    throw(std::exception) 
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
        throw std::logic_error("Tried to open stream for ConfigEntryFile "
                               "that had no filename set.");
}

template <> const char *typeName<bool>() throw() { return "Bool"; }
template <> const char *typeName<char>() throw() { return "Char"; }
template <> const char *typeName<unsigned char>() throw() 
    { return "UnsignedChar"; }
template <> const char *typeName<short>() throw() { return "Short"; }
template <> const char *typeName<unsigned short>() throw() 
    { return "UnsignedShort"; }
template <> const char *typeName<int>() throw() { return "Int"; }
template <> const char *typeName<unsigned int>() throw() 
    { return "UnsignedInt"; }
template <> const char *typeName<long>() throw() { return "Long"; }
template <> const char *typeName<unsigned long>() throw() 
    { return "UnsignedLong"; }
template <> const char *typeName<long long>() throw() { return "LongLong"; }
template <> const char *typeName<unsigned long long>() throw() 
    { return "UnsignedLongLong"; }
template <> const char *typeName<float>() throw() { return "Float"; }
template <> const char *typeName<double>() throw() { return "Double"; }
template <> const char *typeName<long double>() throw() 
    { return "LongDouble"; }
template <> const char *typeName<string>() throw() { return "String"; }

}

#include "ConfigEntryTypeImpl.hh"
namespace simparm {
    template class ConfigEntryType<string>;
    template class ConfigEntryType<bool>;
    template class ConfigEntryType<unsigned long>;
};

#include "ConfigEntryNumericImpl.hh"

namespace simparm {
    template class ConfigEntryNumeric<short int>;
    template class ConfigEntryNumeric<int>;
    template class ConfigEntryNumeric<long int>;
    template class ConfigEntryNumeric<unsigned short int>;
    template class ConfigEntryNumeric<unsigned int>;
    template class ConfigEntryNumeric<unsigned long int>;
    template class ConfigEntryNumeric<float>;
    // ConfigEntryProgress does this: 
    template class ConfigEntryNumeric<double>;
    template class ConfigEntryNumeric<long double>;
};
