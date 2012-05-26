#ifndef SIMPARM_BACKEND_ROOT_H
#define SIMPARM_BACKEND_ROOT_H

#include "NodeBackend.h"
#include "ChildrenList.h"

namespace simparm {
namespace text_stream {

class BackendRoot : public BackendNode {
    virtual const std::string& get_name() const;
    virtual std::ostream* get_print_stream();
    virtual Message::Response send_( const Message& );
    virtual void process_child_command_( const std::string& child, std::istream& rest );
    virtual Mutex* get_mutex();
    virtual void add_child( BackendNode& );
    virtual void remove_child( BackendNode& );
    virtual void declare( std::ostream& );

    bool attached;
    bool should_quit;
    std::ostream* out;
    Mutex mutex;

    ChildrenList< BackendNode > children;
protected:
    virtual void process_command_( const std::string& command, std::istream& rest );
public:
    BackendRoot( std::ostream* );
    ~BackendRoot();
    void processCommand( std::istream& );
    bool received_quit_command() const;
};

}
}

#endif
