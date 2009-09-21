
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

#include "Set.hh"
#include "Entry.hh"
#include "TriggerEntry.hh"
#include <fstream>
#include <sstream>

#include <algorithm>
#include <functional>
#include <string.h>
#include <getopt.h>
#include <vector>
#include <cassert>
#include <map>

using namespace std;

namespace simparm {

Set::Set(std::string name, std::string desc)
: Node(), Object(name, desc),
  showTabbed("showTabbed", false) 
{
    push_back(showTabbed);
}

Set::Set(const Set &o) 
: Node(o), Object(o), showTabbed(o.showTabbed) {
    push_back(showTabbed);
}
Set::~Set() {
}

static bool isEntryBool(const Node &entry) {
   return dynamic_cast<const BoolEntry*>(&entry) != NULL;
}
static bool isTriggerEntry(const Node &entry) {
   return dynamic_cast<const TriggerEntry*>(&entry) != NULL;
}

template <typename Type>
class shared_ptr {
  private:
    pair<Type,int> *ptr;
  public:
    shared_ptr() : ptr( new pair<Type,int>() ) { ptr->second = 1; }
    shared_ptr(const shared_ptr& other) : ptr(other.ptr) 
        { ptr->second++; }
    ~shared_ptr() { ptr->second--; if (ptr->second == 0) delete ptr; }

    Type& operator*() { return ptr->first; }
    const Type& operator*() const { return ptr->first; }
    Type* operator->() { return &ptr->first; }
    const Type* operator->() const { return &ptr->first; }
};

class NodeSerializer : public unary_function<void,Node*> {
  private:
    /** The bool indicates whether help should be given. */
    shared_ptr< list<std::pair< string, bool > > > series;
    shared_ptr< map<string, pair<Node*,int> > > byName;

  public:
    void operator()(Node* o) {
        if ( dynamic_cast<Entry*>(o) != NULL ) {
            string name = o->getName();
            if (byName->find( name ) == byName->end() ) {
                byName->insert( make_pair( name, make_pair((Node*)NULL,0) ) );
                series->push_back( make_pair( name, true ) );
            }

            int count = ( ++ (*byName)[ name ].second );
            stringstream qualified_name;
            qualified_name << count << "." << o->getName();
            series->push_back( make_pair( qualified_name.str(), false ) );

            (*byName)[ o->getName() ].first = o;
            (*byName)[ qualified_name.str() ].first = o;
        }
        for_each( o->getChildren().begin(), o->getChildren().end(),
                  *this );
    }

    class const_iterator {
      private:
        list< pair<string,bool> >::const_iterator base;
        const map<string, pair<Node*,int> >& byName;
      public:
        const_iterator(list< pair<string,bool> >::const_iterator i,
            const map<string, pair<Node*,int> >& byName)
            : base(i), byName(byName)
        {
        }

        const_iterator operator++() { ++base; return *this; }
        const_iterator operator--() { --base; return *this; }
        const_iterator operator++(int) { base++; return *this; }
        const_iterator operator--(int) { base--; return *this; }

        bool operator==(const const_iterator &o) { return o.base == base; }
        bool operator!=(const const_iterator &o) { return o.base != base; }

        string name() { return base->first; }
        bool printHelp() { return base->second; }

        Node& operator*() 
            { map<string, pair<Node*,int> >::const_iterator it =
                byName.find( base->first ); return *it->second.first; }
        Node* operator->() 
            { map<string, pair<Node*,int> >::const_iterator it =
                byName.find( base->first ); return it->second.first; }
    };

    const_iterator begin() const 
        { return const_iterator(series->begin(), *byName); }
    const_iterator end() const 
        { return const_iterator(series->end(), *byName); }
};

class Option {
   private:
      string name;
      Entry *entry;
      const string* default_argument;
      bool gets_argument;
      auto_ptr<string> given_argument;

   public:
      Option(
         const string &name, Entry &e, 
         const string *argument, bool is_opt = false
      )
      : name(name), entry(&e), default_argument(argument), 
         gets_argument(!default_argument || is_opt)
      {
         assert( (gets_argument && !is_opt) || default_argument );
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
         assert( opt.has_arg == required_argument || default_argument );
         opt.flag = NULL;
      }

      void wasMentioned(const char *argument) {
         string used_argument;
         if (!gets_argument || !argument) {
            assert( default_argument != NULL );
            used_argument = *default_argument;
         } else 
            used_argument = argument;

         stringstream ss(" = " + used_argument);
         entry->processCommand( ss );
      }
};

static string s_true = "true", s_false = "false", s_trigger = "1";

string first_alpha_to_lowercase( string s ) {
    string rv = s;
    unsigned int i = 0;

    while ( i < rv.size() && !isalpha( rv[i] ) ) 
        i++;
    if ( i < rv.size() )
        rv[i] = tolower(rv[i]);

    return rv;
}

void collect_args(const Set &c, vector<Option>& options) 
{
   NodeSerializer serializer;
   serializer = for_each( c.getChildren().begin(), c.getChildren().end(),
                          serializer );

   for (NodeSerializer::const_iterator i = serializer.begin(); 
                                             i != serializer.end(); i++)
   {
        Entry *configNode = dynamic_cast<Entry*>( &*i );
        if (configNode == NULL) continue;
        Entry &ce = *configNode;
        if (ce.editable())
        {
         if ( isEntryBool(ce) ) {
            options.push_back(Option(i.name(), ce, &s_true, true));
            if ( first_alpha_to_lowercase(i.name()) != i.name() )
                options.push_back(Option(
                    first_alpha_to_lowercase(i.name()), ce, &s_true, true));
            options.push_back(Option("enable-"+i.name(), ce, &s_true));
            options.push_back(Option("disable-"+i.name(), ce, &s_false));
            options.push_back(Option("set-"+i.name(), ce, &s_true));
            options.push_back(Option("unset-"+i.name(), ce, &s_false));
            options.push_back(Option("no-"+i.name(), ce, &s_false));
            options.push_back(Option("no"+i.name(), ce, &s_false));
         } else {
            const string *defarg = NULL;
            if ( isTriggerEntry(ce) ) 
               defarg = &s_trigger;

            options.push_back(Option(i.name(), ce, defarg));
            if ( first_alpha_to_lowercase(i.name()) != i.name() )
                options.push_back(Option(
                    first_alpha_to_lowercase(i.name()), ce, defarg));
         }
        }
   }
}

int Set::readConfig(int argc, char *argv[]) {
    do {
        vector<Option> options;
        int flag = 0;
        collect_args(*this, options);

        int additional_args = 2;

        struct option longopts[options.size() + additional_args + 1];
        for (unsigned int i = 0; i < options.size(); i++) {
            options[i].writeLongOpt(longopts[i+additional_args]);
            longopts[i+additional_args].flag = &flag;
            longopts[i+additional_args].val = i;
        }

        /* Add help option */
        longopts[0].name = "help";
        longopts[0].has_arg = no_argument;
        longopts[0].flag = &flag;
        longopts[0].val = -2;

        /* Add load option */
        longopts[1].name = "load";
        longopts[1].has_arg = required_argument;
        longopts[1].flag = &flag;
        longopts[1].val = -1;

        /* Write termination */
        memset(longopts + options.size() + additional_args,
               0, sizeof(struct option));

        opterr = 1;
        int longopt_index;
        int rv = getopt_long(argc, argv, "", longopts, &longopt_index);
        if (rv == -1) {
            /* End of options */
            break;
        } else if ( rv == 0 ) {
            if ( flag == -2 ) {
                this->help( cerr );
                return -1;
            } else if ( flag == -1 ) {
                std::ifstream file( optarg );
                while ( file )
                    processCommand( file );
            } else 
                options[flag].wasMentioned(optarg);
        } else if ( rv == '?' ) {
            cerr << PACKAGE_NAME << ": warning: No config option named "
                << argv[optind-1] << endl;
            optind++;
            if (optind >= argc) break;
        } else {
            break;
        }
    } while ( true );

   return optind;
}

void Set::help(ostream &o) const {
   NodeSerializer serializer;
   serializer = for_each( getChildren().begin(), getChildren().end(),
                          serializer );
   for (NodeSerializer::const_iterator i = serializer.begin();
                    i != serializer.end(); i++)
    {
      if ( ! i.printHelp() ) continue;
      Entry *entry = dynamic_cast<Entry*>( &*i );
      if ( entry && entry->viewable() )
         entry->printHelp(o);
    }
}

}
