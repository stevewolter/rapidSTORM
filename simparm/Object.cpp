#include "simparm/Object.h"
#include "simparm/Node.h"
#include "simparm/dummy_ui/fwd.h"
#include "simparm/GUILabelTable.h"

using namespace std;

namespace simparm {

Object::Object(string name, string desc)
: node_( dummy_ui::make_node() ), name(name), desc(desc), is_visible(true), user_level( Beginner )
{
}

Object::Object(string name)
: node_( dummy_ui::make_node() ), name(name), is_visible(true), user_level( Beginner )
{
    const GUILabelTable::Entry& e = GUILabelTable::get_singleton().get_entry( name );
    desc = e.description;
}

Object::Object(const Object& o)
: node_( dummy_ui::make_node() ), name(o.name), desc(o.desc), is_visible(o.is_visible), user_level( o.user_level )
{}

Object::~Object() { 
}

NodeHandle Object::attach_ui( simparm::NodeHandle node ) {
    if ( node ) {
        NodeHandle r = create_hidden_node( node );
        r->initialization_finished();
        return r;
    } else {
        return node;
    }
}

void Object::detach_ui( simparm::NodeHandle node ) {
    node_.reset();
}

NodeHandle Object::create_hidden_node( simparm::NodeHandle node ) { 
    node_ = make_naked_node( node );
    node_->set_description( desc );
    node_->set_visibility( is_visible );
    node_->set_user_level( user_level );
    return node_;
}

NodeHandle Object::make_naked_node( simparm::NodeHandle node ) {
    return node->create_object( name );
}

std::string Object::getName() const { return name; }
std::string Object::getDesc() const { return desc; }

void Object::show() { set_visibility(true); }
void Object::hide() { set_visibility(false); }
void Object::set_visibility( bool arg ) { is_visible = arg; node_->set_visibility( arg ); }
void Object::set_user_level( UserLevel arg ) { user_level = arg; node_->set_user_level( arg ); }


};
