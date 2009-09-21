
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

#if !defined(CONFIGENTRY_HH)
#define CONFIGENTRY_HH

#include <simparm/Object.hh>
#include <list>
#include <iostream>

namespace simparm {
using std::string;
using std::istream;
using std::ostream;

template <typename Type> 
const char *typeName();

class Entry : public Object {
  private:
    std::string getPrefix() const { return "set "; }

  public:
    Attribute<string> help;
    Attribute<bool> invalid, editable, outputOnChange;
    Attribute<int> helpID;

  public:
    Entry(string name = "", string desc = "");
    Entry(const Entry&);
    virtual ~Entry() ;

    void setHelp(const string &help)
        { this->help = help; }
    void setInvalid(const bool &invalid)
        { this->invalid = invalid; }
    void setEditable(const bool &editable)
        { this->editable = editable; }
    void setOutputOnChange(const bool &outputOnChange)
        { this->outputOnChange = outputOnChange; }

    void printHelp(std::ostream &o) const;
};

template <typename TypeOfEntry>
class EntryType : public Entry {
  protected:
    virtual string getTypeDescriptor() const
        { return typeName<TypeOfEntry>() + string("Entry"); }

  public:
    Attribute<TypeOfEntry> value;

    EntryType(string name = "", string desc = "",
                const TypeOfEntry& value = TypeOfEntry());
    EntryType(const EntryType<TypeOfEntry> &entry);
    virtual ~EntryType() ;
    virtual EntryType<TypeOfEntry>* clone() const
        { return new EntryType<TypeOfEntry>(*this); }

    inline const TypeOfEntry &operator()() const { return value(); }
    virtual void setValue(const TypeOfEntry &value)
        { this->value = value; }

    std::list<std::string> printValues() const {
        std::list<std::string> children = Object::printValues();
        children.push_front( getName() + " = " + value.getValueString() );
        return children;
    }

    EntryType<TypeOfEntry> &
        operator=(const EntryType<TypeOfEntry> &entry);
    EntryType<TypeOfEntry> &operator=(const TypeOfEntry &entry) 
 { this->value = entry; return *this; }

    inline bool operator==(const EntryType<TypeOfEntry> &o)
        const
        { return (name() == o.name()) && 
                    (value() == o.value()); }
    inline bool operator!=(const EntryType<TypeOfEntry>
        &entry) const
        { return ! ( (*this) == entry ); }
    inline bool operator==(const TypeOfEntry &value) const
        { return this->value() == value; }
    inline bool operator!=(const TypeOfEntry &value) const
        { return this->value() != value; }
};

typedef EntryType<bool> BoolEntry;
typedef EntryType<string> StringEntry;

}

#endif

