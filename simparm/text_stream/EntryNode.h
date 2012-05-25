#ifndef SIMPARM_TEXT_STREAM_ENTRY_NODE_H
#define SIMPARM_TEXT_STREAM_ENTRY_NODE_H

#include "Node.h"

namespace simparm {
namespace text_stream {

class EntryNode : public Node {
    Attribute<std::string> help;
    Attribute<bool> editable;
    Attribute<std::string> helpID;

    void set_help_id( std::string id ) { helpID = id; }
    void set_help( std::string s ) { help = s; }
    void set_editability( bool editable ) { this->editable = editable; }
public:
    EntryNode( std::string name, std::string type )
        : Node(name,type),
          help("help", ""),
          editable("editable", true),
          helpID("helpID", "")
    {
        add_attribute( editable );
        add_attribute( help );
        add_attribute( helpID );
    }
};

}
}

#endif
