#include "simparm/wx_ui/ProtocolNode.h"
#include <iostream>

namespace simparm {
namespace wx_ui {

ProtocolNode::ProtocolNode( const ProtocolNode& parent, std::string name ) 
: path( parent.path + " in " + name),
  target( parent.target )
{}

ProtocolNode::ProtocolNode( std::ostream* target ) : target(target) {}

void ProtocolNode::protocol( std::string message ) const {
    if ( target )
        *target << path << " " << message << std::endl;
}

}
}
