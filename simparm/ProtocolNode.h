#ifndef SIMPARM_WX_UI_PROTOCOL_NODE_H
#define SIMPARM_WX_UI_PROTOCOL_NODE_H

#include <iosfwd>
#include <string>

namespace simparm {

class ProtocolNode {
    std::string path;
    std::ostream* target;
public:
    ProtocolNode( const ProtocolNode& parent, std::string name );
    ProtocolNode( std::ostream* target );
    void protocol( std::string message ) const;
};

}

#endif
