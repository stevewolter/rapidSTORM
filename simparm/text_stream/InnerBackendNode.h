#ifndef SIMPARM_INNER_BACKEND_NODE_H
#define SIMPARM_INNER_BACKEND_NODE_H

#include "NodeBackend.h"
#include "ChildrenList.h"

#include <boost/bind/bind.hpp>

namespace simparm {
namespace text_stream {

struct InnerBackendNode : public BackendNode {
    std::string name, type;
    ChildrenList< BackendNode > children;
    boost::shared_ptr<BackendNode> parent;
    bool declared;

    FrontendNode* frontend;
    boost::recursive_mutex* tree_mutex;

    std::ostream* get_print_stream();
    Message::Response send_( const Message& m )
        { return parent->send_(m); }
    void process_child_command_( const std::string& child, std::istream& rest );
    const std::string& get_name() const { return name; }
    Mutex* get_mutex() { return tree_mutex; }
    void add_child( BackendNode& t );
    void remove_child( BackendNode& t ); 
    void declare( std::ostream& o );
    std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& );
    void detach_frontend_();

public:
    InnerBackendNode( std::string name, std::string type, FrontendNode& frontend, boost::shared_ptr<BackendNode> parent );
    virtual ~InnerBackendNode(); 
};

}
}

#endif
