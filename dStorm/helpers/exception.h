#ifndef DSTORM_EXCEPTION_H
#define DSTORM_EXCEPTION_H

#include <stdexcept>
#include <simparm/Message.hh>

namespace dStorm {

struct exception {
    virtual simparm::Message get_message(std::string title) const = 0;
};

struct logic_error_message 
: public exception, std::logic_error {
    simparm::Message message;
    logic_error_message(simparm::Message m) 
        : std::logic_error(m.get_message()), message(m) {}
    ~logic_error_message() throw() {}
    simparm::Message get_message(std::string) const
        { return message; }
};

struct runtime_error_message 
: public exception, std::runtime_error {
    simparm::Message message;
    runtime_error_message(simparm::Message m) 
        : std::runtime_error(m.get_message()), message(m) {}
    ~runtime_error_message() throw() {}
    simparm::Message get_message(std::string) const
        { return message; }
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
