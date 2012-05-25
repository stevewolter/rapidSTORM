#ifndef SIMPARM_CMDLINE_UI_CHOICENODE_H
#define SIMPARM_CMDLINE_UI_CHOICENODE_H

#include "EntryNode.h"
#include "OptionTable.h"

namespace simparm {
namespace cmdline_ui {

struct ChoiceNode : public EntryNode
{
    std::map< std::string, Node* > node_lookup;
    BaseAttribute* value;
    std::vector< std::string > names;

    virtual void add_attribute( simparm::BaseAttribute& a ) {
        EntryNode::add_attribute(a);
        if ( a.get_name() == "value" ) value = &a;
    }

    virtual void program_options( OptionTable& t ) {
        std::string choice = * value->get_value();
        std::string choices;
        for ( std::vector< std::string >::const_iterator i = names.begin(); i != names.end(); ++i )
        {
            if ( *i == choice ) choices += "_";
            choices += *i;
            if ( *i == choice ) choices += "_";
            choices += " ";
        }
        this->set_choices( choices );
        EntryNode::program_options( t );
        if ( this->is_visible() ) {
            std::map< std::string, Node* >::iterator i 
                = node_lookup.find( choice );
            if ( i != node_lookup.end() )
                i->second->program_options( t );
        }
    }

    virtual void add_child( Node& o ) {
        Node::add_child(o);
        node_lookup.insert( std::make_pair( o.name, &o ) );
        names.push_back( o.name );
    }

    virtual void remove_child( Node& o ) {
        Node::remove_child( o );
        node_lookup.erase( o.name );
        names.erase( std::remove( names.begin(), names.end(), o.name ) );
    }
public:
    ChoiceNode( std::string name ) : EntryNode(name, OptionTable::Value ), value(NULL) {}
};

}
}

#endif
