
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

#include "ConfigSet.hh"
#include "ConfigEntry.hh"
#include <fstream>
#include <sstream>

#include <algorithm>
#include <functional>
#include <string.h>
#include <getopt.h>
#include <vector>

using namespace std;

namespace simparm {

ConfigSet::ConfigSet() throw() : ConfigObject("INVALID", "INVALID") {}

ConfigSet::ConfigSet(std::string name, std::string desc) throw()
: ConfigObject(name, desc) {}

ConfigSet::ConfigSet(const ConfigSet &o) : ConfigObject(o) {
    for_each( o.children.begin(), o.children.end(), 
        bind1st(mem_fun(&ConfigObject::register_entry), this) );
}
ConfigSet::~ConfigSet() {}

static bool isConfigEntryBool(const ConfigNode &entry) {
   return dynamic_cast<const ConfigEntryBool*>(&entry) != NULL;
}
static bool isConfigEntryTrigger(const ConfigNode &entry) {
   return dynamic_cast<const ConfigEntryTrigger*>(&entry) != NULL;
}

template <typename Type>
class shared_ptr {
  private:
    pair<Type,int> *ptr;
  public:
    shared_ptr() throw() : ptr( new pair<Type,int>() ) { ptr->second = 1; }
    shared_ptr(const shared_ptr& other) throw() : ptr(other.ptr) 
        { ptr->second++; }
    ~shared_ptr() throw() { ptr->second--; if (ptr->second == 0) delete ptr; }

    Type& operator*() throw() { return ptr->first; }
    const Type& operator*() const throw() { return ptr->first; }
    Type* operator->() throw() { return &ptr->first; }
    const Type* operator->() const throw() { return &ptr->first; }
};

class ConfigNodeSerializer : public unary_function<void,ConfigNode*> {
  private:
    shared_ptr< list<string> > series;
    shared_ptr< map<string,ConfigNode*> > byName;

  public:
    void operator()(ConfigNode* o) throw() {
        if (byName->find( o->getName() ) == byName->end() )
            series->push_back(o->getName());
        (*byName)[ o->getName() ] = o;
        for_each( o->getChildren().begin(), o->getChildren().end(),
                  *this );
    }

    class const_iterator {
      private:
        list<string>::const_iterator base;
        const map<string,ConfigNode*>& byName;
      public:
        const_iterator(list<string>::const_iterator i,
            const map<string,ConfigNode*>& byName)
            : base(i), byName(byName) {}

        const_iterator operator++() { ++base; return *this; }
        const_iterator operator--() { --base; return *this; }
        const_iterator operator++(int) { base++; return *this; }
        const_iterator operator--(int) { base--; return *this; }

        bool operator==(const const_iterator &o) { return o.base == base; }
        bool operator!=(const const_iterator &o) { return o.base != base; }

        const pair<const string,ConfigNode*>& operator*() const
            { map<string,ConfigNode*>::const_iterator it =
                byName.find( *base ); return *it; }
        const pair<const string,ConfigNode*>* operator->() const
            { map<string,ConfigNode*>::const_iterator it =
                byName.find( *base ); return &(*it); }
    };

    const_iterator begin() const 
        { return const_iterator(series->begin(), *byName); }
    const_iterator end() const 
        { return const_iterator(series->end(), *byName); }
};

class Option {
   private:
      string name;
      ConfigEntry *entry;
      const string* default_argument;
      bool gets_argument;
      auto_ptr<string> given_argument;

   public:
      Option(
         const string &name, ConfigEntry &e, 
         const string *argument, bool is_opt = false
      )
      : name(""), entry(&e), default_argument(argument), 
         gets_argument(!default_argument || is_opt)
      {
         for (string::const_iterator l = name.begin(); l != name.end(); l++)
            if ( ! isspace( *l ) )
               this->name += *l;
      }

      Option(const Option &o) 
      : name(o.name), entry(o.entry), default_argument(o.default_argument),
        gets_argument(o.gets_argument), given_argument(NULL)
      {}

      void writeLongOpt(struct option &opt) {
         opt.name = name.c_str();
         opt.has_arg = 
              (!gets_argument) ? no_argument
            : (default_argument) ? optional_argument
            : required_argument;
         opt.flag = NULL;
      }

      void wasMentioned(const char *argument) {
         string used_argument;
         if (!gets_argument || !argument)
            used_argument = *default_argument;
         else 
            used_argument = argument;

         stringstream ss(used_argument);
         entry->readValue(ss);
      }
};

static string s_true = "true", s_false = "false", s_trigger = "trigger";

void collect_args(const ConfigSet &c, vector<Option>& options) 
{
   ConfigNodeSerializer serializer;
   serializer = for_each( c.getChildren().begin(), c.getChildren().end(),
                          serializer );

   for (ConfigNodeSerializer::const_iterator i = serializer.begin(); 
                                             i != serializer.end(); i++)
   {
        ConfigEntry *configNode = dynamic_cast<ConfigEntry*>(i->second);
        if (configNode == NULL) continue;
        ConfigEntry &ce = *configNode;
        if (ce.editable())
        {
         if ( isConfigEntryBool(ce) ) {
            options.push_back(Option(ce.name(), ce, &s_true, true));
            string first_lower = ce.name();
            first_lower[0] = tolower(first_lower[0]);
            options.push_back(Option(first_lower, ce, &s_true, true));
            options.push_back(Option("enable-"+ce.name(), ce, &s_true));
            options.push_back(Option("disable-"+ce.name(), ce, &s_false));
            options.push_back(Option("set-"+ce.name(), ce, &s_true));
            options.push_back(Option("unset-"+ce.name(), ce, &s_false));
            options.push_back(Option("no-"+ce.name(), ce, &s_false));
            options.push_back(Option("no"+ce.name(), ce, &s_false));
         } else {
            const string *defarg = NULL;
            if ( isConfigEntryTrigger(ce) ) 
               defarg = &s_trigger;

            options.push_back(Option(ce.name(), ce, defarg));
            string first_lower = ce.name();
            first_lower[0] = tolower(first_lower[0]);
            options.push_back(Option(first_lower, ce, defarg));
         }
        }
   }
}

bool ConfigSet::readConfig(int argc, char *argv[]) throw() {
    do {
        vector<Option> options;
        int flag = 0;
        collect_args(*this, options);

        struct option longopts[options.size() + 2];
        for (unsigned int i = 0; i < options.size(); i++) {
            options[i].writeLongOpt(longopts[i+1]);
            longopts[i+1].flag = &flag;
            longopts[i+1].val = i;
        }

        /* Add help option */
        longopts[0].name = "help";
        longopts[0].has_arg = no_argument;
        longopts[0].flag = &flag;
        longopts[0].val = 0;

        /* Write termination */
        memset(longopts + options.size(), 0, sizeof(struct option));

        opterr = 1;
        int longopt_index;
        int rv = getopt_long(argc, argv, "", longopts, &longopt_index);
        if (rv == -1) {
            /* End of options */
            break;
        } else if ( rv == 0 ) {
            if ( flag == 0 ) {
                this->help( cerr );
                exit(1);
            } else 
                options[flag].wasMentioned(optarg);
        } else if ( rv == '?' ) {
            cerr << PACKAGE_NAME << ": warning: No config option named "
                << argv[optind-1] << endl;
            optind++;
        } /*else {
            char *config_fname;
            ifstream config_fin;

            config_fname = argv[optind];
            config_fin.open(config_fname);
            while ( ! config_fin.eof() )
                this->processCommand( config_fin );

            if ( ! config_fin.eof()) {
                cerr << "Error reading config file" << endl;
                return 1;
            }
            config_fin.close();
            optind++;
        }*/
    } while ( true );

   return 0;
}

void ConfigSet::help(ostream &o) const throw() {
   ConfigNodeSerializer serializer;
   serializer = for_each( getChildren().begin(), getChildren().end(),
                          serializer );
   for (ConfigNodeSerializer::const_iterator i = serializer.begin();
                    i != serializer.end(); i++)
    {
      ConfigEntry *entry = dynamic_cast<ConfigEntry*>(i->second);
      if ( entry && entry->viewable() )
         entry->printHelp(o);
    }
}

}
