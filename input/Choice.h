#ifndef DSTORM_INPUT_CHAINCHOICE_H
#define DSTORM_INPUT_CHAINCHOICE_H

#include "input/Link.h"
#include "simparm/ManagedChoiceEntry.h"

namespace dStorm {
namespace input {

class Choice
: public Link
{
  public:
    class LinkAdaptor : public simparm::Choice {
        std::unique_ptr<input::Link> _link;
        Link::Connection connection;

      public:
        LinkAdaptor( std::unique_ptr<input::Link> l );
        ~LinkAdaptor();
        Link& link() { return *_link; }
        const Link& link() const { return *_link; }
        LinkAdaptor* clone() const {
            std::unique_ptr<LinkAdaptor> rv( new LinkAdaptor( std::unique_ptr<Link>(_link->clone()) ) );
            return rv.release();
        }
        void connect( input::Choice& c ) {
            connection = _link->notify( boost::bind(
                &input::Choice::traits_changed, &c, _1, _link.get() ) );
        }

        std::string getName() const { return _link->name(); }
        void attach_ui( simparm::NodeHandle at ) { _link->registerNamedEntries( at ); }
        void detach_ui( simparm::NodeHandle at ) { throw std::logic_error("Not implemented"); }
    };

  protected:
    typedef simparm::ManagedChoiceEntry<LinkAdaptor>::iterator iterator;
    simparm::ManagedChoiceEntry<LinkAdaptor> choices;
    boost::shared_ptr<MetaInfo> my_traits;
    bool auto_select, will_publish_traits;

    void publish_traits_locked();
    void publish_traits();
    void traits_changed( TraitsRef, Link* );

    simparm::BaseAttribute::Connection value_change_listen;

  public:
    Choice(std::string name, bool auto_select);
    Choice(const Choice&);
    ~Choice();
    
    BaseSource* makeSource() OVERRIDE;
    Choice* clone() const OVERRIDE;
    void registerNamedEntries( simparm::NodeHandle ) OVERRIDE;
    std::string name() const OVERRIDE { return choices.getName(); }
    void publish_meta_info() OVERRIDE;

    void add_choice( std::unique_ptr<Link> r );

    void insert_new_node( std::unique_ptr<Link> ) OVERRIDE;

    void set_help_id( std::string id ) { choices.setHelpID( id ); }

  private:
    std::string description() const { return choices.getDesc(); }
};

}
}

#endif
