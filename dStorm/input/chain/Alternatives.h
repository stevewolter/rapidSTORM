#ifndef DSTORM_INPUT_ALTERNATIVES_H
#define DSTORM_INPUT_ALTERNATIVES_H

#include "Choice.h"

namespace dStorm {
namespace input {
namespace chain {

class Alternatives
: public Choice 
{
    class UpstreamCollector;
    class UpstreamLink;
    std::auto_ptr<UpstreamCollector> collector;

  public:
    Alternatives(std::string name, std::string desc, bool auto_select);
    Alternatives(const Alternatives&);
    virtual Alternatives* clone() const { return new Alternatives(*this); }
    ~Alternatives();

    void set_more_specialized_link_element( Link* );
    void add_choice( std::auto_ptr<Link> );
    void insert_new_node( std::auto_ptr<Link> link, Place p );
    void registerNamedEntries( simparm::Node& );
    void publish_meta_info();
};

}
}
}

#endif
