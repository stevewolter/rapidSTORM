#ifndef SIMPARM_CMDLINE_UI_ROOTNODE_H
#define SIMPARM_CMDLINE_UI_ROOTNODE_H

#include "Node.h"

namespace simparm {
namespace cmdline_ui {

class RootNode : public Node {
public:
    RootNode();
    void parse_command_line( int argc, char **argv );
    Message::Response send( Message& m ) const;
    void print_help();
};

}
}

#endif
