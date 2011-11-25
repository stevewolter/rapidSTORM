#ifndef DSTORM_EXCEPTION_H
#define DSTORM_EXCEPTION_H

#include <stdexcept>
#include <simparm/Message.hh>

namespace dStorm {

struct exception {
    virtual simparm::Message get_message(std::string title) const = 0;
};

class logic_error : public exception, std::logic_error {
  public:
    std::string help_id;
    logic_error(std::string reason, const std::string& error_id) 
        : std::logic_error(reason), help_id(error_id) {}
    ~logic_error() throw() {}

    simparm::Message get_message(std::string title) const {
        simparm::Message m(title, what());
        m.helpID = help_id;
        return m;
    }
};

class runtime_error : public std::runtime_error {
  public:
    std::string help_id;
    runtime_error(std::string reason, const std::string& error_id) 
        : std::runtime_error(reason), help_id(error_id) {}
    ~runtime_error() throw() {}

    simparm::Message get_message(std::string title) const {
        simparm::Message m(title, what());
        m.helpID = help_id;
        return m;
    }
};

/** Abort without further explanation. 
 *  Use when error has already been printed. */
class abort {};

}

#endif
