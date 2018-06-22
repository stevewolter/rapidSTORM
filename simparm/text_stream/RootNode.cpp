#include "RootNode.h"
#include "BackendRoot.h"

using namespace std;

namespace simparm {
namespace text_stream {

RootNode::RootNode( std::ostream* o ) 
: Node("IO","IO"),
  root_backend( new BackendRoot(o, false) )
{
    set_backend_node( std::auto_ptr<BackendNode>(root_backend) );
}

RootNode::~RootNode() {}

bool RootNode::received_quit_command() const {
    return root_backend->received_quit_command();
}

void RootNode::processCommand( std::istream& i ) {
    return root_backend->processCommand( i );
}

}
}
