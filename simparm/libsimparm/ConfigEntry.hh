
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

#include "ConfigObject.hh"
#include <list>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>

namespace simparm {
using std::string;
using std::istream;
using std::ostream;

template <typename Type> 
const char *typeName() throw();

class ConfigEntry : public virtual ConfigObject {
  private:
    std::string getPrefix() const throw() { return "set "; }
  public:
    enum UserLevel { Beginner = 10, Intermediate = 20, 
                     Expert = 30, Debug = 40 };
    friend istream& operator>>(istream &i, UserLevel& ul)
        { int v; i >> v; ul = (UserLevel)v; return i; }

  public:
    ConfigAttribute<string> help;
    ConfigAttribute<bool> invalid, viewable, editable, outputOnChange;
    ConfigAttribute<UserLevel> userLevel;

  public:
    ConfigEntry(string name = "", string desc = "") throw();
    ConfigEntry(const ConfigEntry&) throw();
    virtual ~ConfigEntry() throw();

    void setHelp(const string &help) throw()
        { this->help = help; }
    void setInvalid(const bool &invalid) throw()
        { this->invalid = invalid; }
    void setViewable(const bool &viewable) throw()
        { this->viewable = viewable; }
    void setEditable(const bool &editable) throw()
        { this->editable = editable; }
    void setOutputOnChange(const bool &outputOnChange) throw()
        { this->outputOnChange = outputOnChange; }
    void setUserLevel(UserLevel level) throw()
        { this->userLevel = level; }

    virtual istream& readValue(istream& i) = 0;

    void printHelp(std::ostream &o) const throw();
};

template <typename EntryType>
class ConfigEntryType : public ConfigEntry {
  protected:
    virtual string getTypeDescriptor() const throw()
        { return string("ConfigEntry") + typeName<EntryType>(); }

  public:
    ConfigAttribute<EntryType> value;

    ConfigEntryType(string name = "", string desc = "",
                const EntryType& value = EntryType()) throw();
    ConfigEntryType(const ConfigEntryType<EntryType> &entry) throw();
    virtual ~ConfigEntryType() throw();
    virtual ConfigEntryType<EntryType>* clone() const
        { return new ConfigEntryType<EntryType>(*this); }

    inline const EntryType &operator()() const throw() { return value(); }
    virtual void setValue(const EntryType &value)
        { this->value = value; }

    std::list<std::string> printValues() const throw() {
        std::list<std::string> children = ConfigObject::printValues();
        children.push_front( getName() + " = " + value.getValueString() );
        return children;
    }

    ConfigEntryType<EntryType> &
        operator=(const ConfigEntryType<EntryType> &entry) throw();
    ConfigEntryType<EntryType> &operator=(const EntryType &entry) 
        throw() { this->value = entry; return *this; }

    inline bool operator==(const ConfigEntryType<EntryType> &o)
        const throw()
        { return (name() == o.name()) && 
                    (value() == o.value()); }
    inline bool operator!=(const ConfigEntryType<EntryType>
        &entry) const throw()
        { return ! ( (*this) == entry ); }
    inline bool operator==(const EntryType &value) const throw()
        { return this->value() == value; }
    inline bool operator!=(const EntryType &value) const throw()
        { return this->value() != value; }

    istream& readValue(istream& i);
};

typedef ConfigEntryType<bool> ConfigEntryBool;
typedef ConfigEntryType<string> ConfigEntryString;

template <typename NumType>
class ConfigEntryNumeric : public ConfigEntryType<NumType> {
  private:
    class MinMaxWatcher;
    std::auto_ptr< MinMaxWatcher > watcher;
    class IncrementWatcher;
    std::auto_ptr< IncrementWatcher > inc_watcher;
  public:
    ConfigEntryNumeric(string name = "", string desc = "", 
                        NumType value = 0, NumType increment = 0) throw();
    ConfigEntryNumeric(const ConfigEntryNumeric<NumType> &entry) throw();
    ~ConfigEntryNumeric() throw();
    virtual ConfigEntryNumeric<NumType>* clone() const
        { return new ConfigEntryNumeric<NumType>(*this); }

    ConfigEntryType<NumType> &operator=(NumType entry) 
        throw() { this->value = entry; return *this; }

    ConfigAttribute<bool> hasMin, hasMax;
    ConfigAttribute<NumType> increment, min, max;

    void setMin(NumType new_min) throw() { min = new_min; hasMin = true; }
    void setMax(NumType new_max) throw() { max = new_max; hasMax = true; }
    void setNoMin() throw() { hasMin = false; }
    void setNoMax() throw() { hasMax = false; }
    void setIncrement(NumType new_increment) throw() 
        { increment = new_increment; }
};

typedef ConfigEntryNumeric<double> ConfigEntryDouble;
typedef ConfigEntryNumeric<unsigned long> ConfigEntryUnsignedLong;
typedef ConfigEntryNumeric<long> ConfigEntryLong;

class ConfigEntryTrigger : public ConfigEntryType<unsigned long> {
  protected:
    /* For classes deriving from ConfigEntryTrigger: 
        * This method is called every time the trigger is
        * pulled. Return true if the trigger event was
        * processed and the trigger should be untriggered. */
    virtual bool wasTriggered() { return false; }

    virtual string getTypeDescriptor() const throw()
        { return string("ConfigEntryTrigger"); }

  public:
    ConfigEntryTrigger(const ConfigEntryTrigger &entry) throw();
    ConfigEntryTrigger(string name, string desc) throw();
    virtual ~ConfigEntryTrigger() throw() {}
    virtual ConfigEntryTrigger* clone() const
        { return new ConfigEntryTrigger(*this); }

    unsigned long pending() const throw() { return value; }
    bool triggered() const throw() { return value; }

    void trigger() throw() { value += 1; }
    void untrigger() throw() { value -= 1; }
    void setTrigger(bool trigger) 
        { if (trigger) this->trigger(); else untrigger(); }

    ConfigEntryTrigger &operator=(bool triggered) throw()
        { setTrigger(triggered); return *this; }

    bool operator==(const bool &triggered) const throw()
        { return this->triggered() == triggered; }
    bool operator!=(const bool &triggered) const
        { return this->triggered() != triggered; }

    istream& readValue(istream& i);
};

class ConfigEntryProgress : public ConfigEntryDouble {
  protected:
    virtual const char* get_typeName() const { return "Progress"; }
    class ASCII_Progress_Shower;
    std::auto_ptr<ASCII_Progress_Shower> display;

  public:
    virtual ~ConfigEntryProgress() throw();
    ConfigEntryProgress(const ConfigEntryProgress &entry);
    ConfigEntryProgress(string name, string desc,
                        double value = 0, double increment = 0.01);
    virtual ConfigEntryProgress* clone() const
        { return new ConfigEntryProgress(*this); }

    void makeASCIIBar(std::ostream &where);
    void stopASCIIBar();
};

class ConfigEntryFile : public ConfigEntryType<string> {
  private:
    virtual const char* get_typeName() const { return "File"; }
    std::auto_ptr<std::ofstream> openedOStream;
    std::auto_ptr<std::ifstream> openedIStream;
    std::ostream* out_stream;
    std::istream* in_stream;

  public:
   ConfigEntryFile(const ConfigEntryFile &entry);
   ConfigEntryFile(string name, string desc, string value = "");
   ~ConfigEntryFile() throw();
    virtual ConfigEntryFile* clone() const
        { return new ConfigEntryFile(*this); }

   virtual const char *get_type() const;
   ConfigEntryFile &operator=(const string &);

   operator bool() const;

   std::ostream& getOutputStream(bool append = false) 
    throw(std::exception);
   std::istream& getInputStream() throw(std::exception);
};

}

#include "ConfigEntryChoice.hh"

#endif

