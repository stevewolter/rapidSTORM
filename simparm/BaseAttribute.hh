#ifndef SIMPARM_BASE_ATTRIBUTE_HH
#define SIMPARM_BASE_ATTRIBUTE_HH

#include <boost/signals2/slot.hpp>
#include <boost/signals2/connection.hpp>
#include <dStorm/helpers/nocopy_ptr.hpp>
#include <memory>

namespace simparm {

struct BaseAttribute {
    typedef std::auto_ptr< boost::signals2::scoped_connection > Connection;
    typedef dStorm::nocopy_ptr< boost::signals2::scoped_connection > ConnectionStore;
    typedef boost::signals2::slot<void()> Listener;

    virtual ~BaseAttribute() {}
    virtual std::string get_name() const = 0;
    virtual std::string get_value() const = 0;
    virtual void set_value(std::string command, std::istream&) = 0;
    virtual Connection notify_on_value_change( boost::signals2::slot<void()> ) = 0;
};

}

#endif
