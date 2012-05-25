#ifndef SIMPARM_TEXT_STREAM_TAB_NODE_H
#define SIMPARM_TEXT_STREAM_TAB_NODE_H

namespace simparm {
namespace text_stream {

class TabNode : public Node {
    simparm::Attribute<bool> showTabbed;
public:
    TabNode( std::string name, bool show_tabbed ) 
        : Node(name, "Set"), showTabbed("showTabbed", show_tabbed) 
    {
        add_attribute( showTabbed );
    }
};

}
}

#endif
