#ifndef SIMPARM_TEXT_STREAM_ROOT_NODE_HH
#define SIMPARM_TEXT_STREAM_ROOT_NODE_HH

#include <iostream>
#include "simparm/text_stream/Node.h"

namespace simparm {
namespace text_stream {

struct BackendRoot;

class RootNode : public Node {
    BackendRoot* const root_backend;
    
  public:
    RootNode(std::ostream* out = NULL);
    ~RootNode();

    bool received_quit_command() const;
    void processCommand( std::istream& );
};

}
}

#endif
