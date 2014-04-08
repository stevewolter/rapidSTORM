#include "simparm/wx_ui/VisibilityNode.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "simparm/wx_ui/lambda.h"

namespace simparm {
namespace wx_ui {

bool VisibilityNode::is_visible() const { 
    return my_user_level <= desired_user_level && backend_visible; 
}

void VisibilityNode::desired_user_level_changed( UserLevel previous, UserLevel current ) {
    assert( previous == desired_user_level );
    bool was_visible = is_visible();
    desired_user_level = current;
    if ( is_visible() != was_visible ) visibility_changed( is_visible() );
}

VisibilityNode::VisibilityNode( VisibilityControl& vc )
: backend_visible(true), my_user_level(Beginner), desired_user_level( vc.current_user_level() ),
    connection_to_visibility_control( vc.connect( boost::bind( &VisibilityNode::desired_user_level_changed, this, _1, _2 ) ) ) 
{
}

void VisibilityNode::add_group( boost::shared_ptr< Window > g ) {
    visibility_changed.connect( boost::bind( &Window::node_changed_visibility, g, _1 ) );
    if ( ! is_visible() )
        g->node_changed_visibility( false );
}

void VisibilityNode::set_visibility( bool v ) {
    bool was_visible = is_visible();
    backend_visible = v;
    if ( is_visible() != was_visible ) visibility_changed( is_visible() );
}

void VisibilityNode::set_user_level( UserLevel l ) {
    bool was_visible = is_visible();
    my_user_level = l;
    if ( is_visible() != was_visible ) visibility_changed( is_visible() );
}

}
}
