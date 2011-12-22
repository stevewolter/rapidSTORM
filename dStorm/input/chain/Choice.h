#ifndef DSTORM_INPUT_CHAINCHOICE_H
#define DSTORM_INPUT_CHAINCHOICE_H

#include "Link.h"
#include <simparm/ChoiceEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace input {
namespace chain {

class Choice
: public Link, public simparm::Listener
{
  protected:
    class LinkAdaptor {
        simparm::Object node;
        std::auto_ptr<input::chain::Link> _link;

      public:
        LinkAdaptor( std::auto_ptr<input::chain::Link> l );
        ~LinkAdaptor();
        simparm::Node& getNode() { return node[_link->name()]; }
        const simparm::Node& getNode() const { return node[_link->name()]; }
        operator simparm::Node&() { return getNode(); }
        operator const simparm::Node&() const { return getNode(); }
        Link& link() { return *_link; }
        const Link& link() const { return *_link; }
        void registerNamedEntries() {
            _link->registerNamedEntries( node );
        }
        LinkAdaptor* clone() const {
            std::auto_ptr<LinkAdaptor> rv( new LinkAdaptor( std::auto_ptr<Link>(_link->clone()) ) );
            rv->registerNamedEntries();
            return rv.release();
        }
    };

    simparm::NodeChoiceEntry<LinkAdaptor> choices;
    boost::shared_ptr<MetaInfo> my_traits;
    bool auto_select;

    void publish_traits();

  protected:
    virtual void operator()(const simparm::Event&);
  public:
    typedef simparm::NodeChoiceEntry<Link> ChoiceEntry;

    Choice(std::string name, std::string desc, bool auto_select);
    Choice(const Choice&);
    ~Choice();
    
    virtual void traits_changed( TraitsRef, Link* );

    BaseSource* makeSource();
    Choice* clone() const;
    void registerNamedEntries( simparm::Node& );
    std::string name() const { return choices.getName(); }
    std::string description() const { return choices.getDesc(); }
    void publish_meta_info();

    void add_choice( std::auto_ptr<Link> );

    void insert_new_node( std::auto_ptr<Link>, Place );

    void set_help_id( std::string id ) { choices.helpID = id; }

    operator const simparm::Node&() const { return choices; }
    operator simparm::Node&() { return choices; }
};

}
}
}

#endif
