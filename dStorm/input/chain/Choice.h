#ifndef DSTORM_INPUT_CHAINCHOICE_H
#define DSTORM_INPUT_CHAINCHOICE_H

#include "Link.h"
#include <simparm/ChoiceEntry.hh>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace input {
namespace chain {

class Choice
: public Link, public simparm::NodeChoiceEntry<Link>, 
  public simparm::Listener
{
    boost::shared_ptr<MetaInfo> my_traits;
    bool auto_select;
    boost::ptr_vector< Link > choices;

    Choice::AtEnd publish_traits();

  protected:
    virtual void operator()(const simparm::Event&);
  public:
    typedef simparm::NodeChoiceEntry<Link> ChoiceEntry;

    Choice(std::string name, std::string desc, bool auto_select);
    Choice(const Choice&);
    ~Choice();
    
    virtual AtEnd traits_changed( TraitsRef, Link* );

    virtual BaseSource* makeSource();
    virtual Choice* clone() const;
    virtual simparm::Node& getNode();

    void add_choice( std::auto_ptr<Link> );

    void insert_new_node( std::auto_ptr<Link>, Place );
    Link& get_first_link() { return choices[0]; }

    operator const simparm::Node&() const { return *this; }
    operator simparm::Node&() { return *this; }
};

}
}
}

#endif
