#ifndef DSTORM_INPUT_CHAINCHOICE_H
#define DSTORM_INPUT_CHAINCHOICE_H

#include "input/Link.h"
#include "simparm/ManagedChoiceEntry.h"

namespace dStorm {
namespace input {

template <typename Type>
class Choice
: public Link<Type>
{
  public:
    class LinkAdaptor : public simparm::Choice {
        std::unique_ptr<Link<Type>> _link;
        typename Link<Type>::Connection connection;

      public:
        LinkAdaptor( std::unique_ptr<Link<Type>> l ) : _link(std::move(l)) {}
        ~LinkAdaptor() { _link.reset(); }
        Link<Type>& link() { return *_link; }
        const Link<Type>& link() const { return *_link; }
        LinkAdaptor* clone() const {
            std::unique_ptr<LinkAdaptor> rv( new LinkAdaptor( std::unique_ptr<Link<Type>>(_link->clone()) ) );
            return rv.release();
        }
        void connect( input::Choice<Type>& c ) {
            connection = _link->notify( boost::bind(
                &input::Choice<Type>::traits_changed, &c, _1, _link.get() ) );
        }

        std::string getName() const { return _link->name(); }
        void attach_ui( simparm::NodeHandle at ) { _link->registerNamedEntries( at ); }
        void detach_ui( simparm::NodeHandle at ) { throw std::logic_error("Not implemented"); }
    };

  protected:
    typedef typename simparm::ManagedChoiceEntry<LinkAdaptor>::iterator iterator;
    simparm::ManagedChoiceEntry<LinkAdaptor> choices;
    boost::shared_ptr<MetaInfo> my_traits;
    bool auto_select, will_publish_traits;

    void publish_traits_locked();
    void publish_traits();
    void traits_changed( typename Link<Type>::TraitsRef, Link<Type>* );

    simparm::BaseAttribute::Connection value_change_listen;

  public:
    Choice(std::string name, bool auto_select);
    Choice(const Choice&);
    
    Source<Type>* makeSource() OVERRIDE;
    Choice* clone() const OVERRIDE;
    void registerNamedEntries( simparm::NodeHandle ) OVERRIDE;
    std::string name() const OVERRIDE { return choices.getName(); }
    void publish_meta_info() OVERRIDE;

    void add_choice( std::unique_ptr<Link<Type>> r );

    void set_help_id( std::string id ) { choices.setHelpID( id ); }

  private:
    std::string description() const { return choices.getDesc(); }
};

}
}

#endif
