#ifndef DSTORM_EXCEPTION_H
#define DSTORM_EXCEPTION_H

#include <stdexcept>
#include <simparm/Message.hh>

namespace dStorm {
class runtime_error : public std::runtime_error {
  public:
    int help_id;
    runtime_error(std::string reason, int error_id) 
        : std::runtime_error(reason), help_id(error_id) {}

    simparm::Message get_message(std::string title) {
        simparm::Message m(title, what());
        m.helpID = help_id;
        return m;
    }
};

}

#endif
