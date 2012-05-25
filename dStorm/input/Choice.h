#ifndef DSTORM_INPUT_CHAINCHOICE_H
#define DSTORM_INPUT_CHAINCHOICE_H

#include "Link.h"
#include <simparm/ManagedChoiceEntry.h>

namespace dStorm {
namespace input {

class Choice
: public Link
{
  public:
    class LinkAdaptor {
        std::auto_ptr<input::Link> _link;
        Choice *target;
        Link::Connection connection;

      public:
        LinkAdaptor( std::auto_ptr<input::Link> l );
        ~LinkAdaptor();
        Link& link() { return *_link; }
        const Link& link() const { return *_link; }
        LinkAdaptor* clone() const {
            std::auto_ptr<LinkAdaptor> rv( new LinkAdaptor( std::auto_ptr<Link>(_link->clone()) ) );
            return rv.release();
        }
        void connect( Choice& c ) {
            connection = _link->notify( boost::bind(
                &Choice::traits_changed, &c, _1, _link.get() ) );
        }

        std::string getName() const { return _link->name(); }
        void attach_ui( simparm::NodeHandle at ) { _link->registerNamedEntries( at ); }
        void detach_ui( simparm::NodeHandle at ) { throw std::logic_error("Not implemented"); }
    };

  protected:
    typedef simparm::ManagedChoiceEntry<LinkAdaptor>::iterator iterator;
    simparm::ManagedChoiceEntry<LinkAdaptor> choices;
    boost::shared_ptr<MetaInfo> my_traits;
    bool auto_select;

    void publish_traits_locked();
    void publish_traits();
    void traits_changed( TraitsRef, Link* );

    simparm::BaseAttribute::Connection value_change_listen;

  public:
    Choice(std::string name, std::string desc, bool auto_select);
    Choice(const Choice&);
    ~Choice();
    
    BaseSource* makeSource();
    Choice* clone() const;
    void registerNamedEntries( simparm::NodeHandle );
    std::string name() const { return choices.getName(); }
    std::string description() const { return choices.getDesc(); }
    void publish_meta_info();

    void add_choice( std::auto_ptr<Link> );

    void insert_new_node( std::auto_ptr<Link>, Place );

    void set_help_id( std::string id ) { choices.setHelpID( id ); }
};

}
}

#endif
