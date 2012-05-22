#ifndef SIMPARM_BASE_ATTRIBUTE_HH
#define SIMPARM_BASE_ATTRIBUTE_HH

#include <boost/signals2/slot.hpp>

namespace simparm {

struct BaseAttribute {
    virtual ~BaseAttribute() {}
    virtual std::string get_name() const = 0;
    virtual std::string get_value() const = 0;
    virtual void set_value(std::string command, std::istream&) = 0;
    virtual void notify_on_value_change( boost::signals2::slot<void()> ) = 0;
};

}

#endif
