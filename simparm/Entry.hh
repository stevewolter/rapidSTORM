
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

#include "Entry_decl.hh"
#include "Object.hh"
#include <list>
#include <iostream>
#include <boost/utility.hpp>
#include <boost/utility/base_from_member.hpp>
#include "BoostOptional.hh"
#include "default_value.hh"
#include "MinMaxWatcher.hh"
#include "Attributes.hh"

namespace simparm {
using std::string;
using std::istream;
using std::ostream;

template <typename Type> 
const char *typeName();

class BasicEntry : public Object {
  public:
    Attribute<string> help;
    Attribute<bool> invalid, editable, outputOnChange;
    Attribute<string> helpID;

protected:
    NodeRef create_hidden_node( simparm::Node& );
public:
    BasicEntry(string name, string desc = "");
    BasicEntry(const BasicEntry&);
    ~BasicEntry() ;

    void setHelp(const string &help)
        { this->help = help; }
    void setInvalid(const bool &invalid)
        { this->invalid = invalid; }
    void setEditable(const bool &editable)
        { this->editable = editable; }
    void setOutputOnChange(const bool &outputOnChange)
        { this->outputOnChange = outputOnChange; }

    void printHelp(ostream &) const;
    void processCommand( std::istream& i );
    bool has_child_named(const std::string& name) const;
    bool is_entry() const { return true; }
    BasicEntry& get_entry() { return *this; }
};

template<typename ValueField>
struct Attributes<bool,ValueField,void> : private boost::noncopyable {
    Attributes( Attribute<ValueField>& ) {}
    Attributes( const Attributes&, Attribute<ValueField>& ) {}
    Attributes& operator=( const Attributes& ) { return *this; }
    void registerNamedEntries( simparm::Node& ) {}
};

template<typename ValueField>
struct Attributes<std::string,ValueField,void> : private boost::noncopyable {
    Attributes( Attribute<ValueField>& ) {}
    Attributes( const Attributes&, Attribute<ValueField>& ) {}
    Attributes& operator=( const Attributes& ) { return *this; }
    void registerNamedEntries( simparm::Node& ) {}
};

template <typename Type, typename ValueField>
struct Attributes<Type, ValueField, typename boost::enable_if< boost::is_fundamental<Type> >::type >
: private boost::noncopyable
{
    typedef typename add_boost_optional<ValueField>::type bound_type;
  public:
    Attribute<ValueField> increment;
    Attribute< bound_type > min, max;

    Attributes( Attribute<ValueField>& value );
    Attributes( const Attributes&, Attribute<ValueField>& value );
    Attributes& operator=( const Attributes& o ) {
        increment = o.increment();
        min = o.min();
        max = o.max();
        return *this;
    }
    void registerNamedEntries( simparm::Node& );

 private:
    BoundWatcher< bound_type,ValueField,true> value_above_min;
    BoundWatcher< ValueField,bound_type,true> max_above_value;
    BoundWatcher< bound_type,ValueField,false> value_below_max;
    BoundWatcher< ValueField,bound_type,false> min_below_value;
};

template <typename TypeOfEntry>
class Entry 
: public BasicEntry, public boost::base_from_member< Attribute<TypeOfEntry> >,
  public Attributes<TypeOfEntry>
{
  protected:
    NodeRef create_hidden_node( simparm::Node& );

  public:
    typedef TypeOfEntry value_type;
    typedef TypeOfEntry result_type;

    Attribute<TypeOfEntry>& value;

    Entry(string name, string desc = "",
                const TypeOfEntry& value = TypeOfEntry());
    Entry(const Entry<TypeOfEntry> &entry);
    ~Entry() ;
    Entry<TypeOfEntry>* clone() const
        { return new Entry<TypeOfEntry>(*this); }

    inline const TypeOfEntry &operator()() const { return value(); }
    void setValue(const TypeOfEntry &value)
        { this->value = value; }

    Entry<TypeOfEntry> &
        operator=(const Entry<TypeOfEntry> &entry);
    Entry<TypeOfEntry> &operator=(const TypeOfEntry &entry) 
 { this->value = entry; return *this; }

    bool operator==(const Entry<TypeOfEntry> &o) const;
    inline bool operator!=(const Entry<TypeOfEntry> &entry) const;
    inline bool operator==(const TypeOfEntry &value) const;
    inline bool operator!=(const TypeOfEntry &value) const;
};

typedef Entry<bool> BoolEntry;
typedef Entry<string> StringEntry;

}

#endif

