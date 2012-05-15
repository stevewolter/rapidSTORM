#include "TreeCallback.hh"
#include "Node.hh"

namespace simparm {

void TreeListener::receive_changes_from_subtree( Node& node )
{
    receive_changes_from( node );
    for ( Node::iterator i = node.begin(); i != node.end(); ++i ) {
 	receive_changes_from_subtree(*i);
    }
}

void TreeListener::add_new_children( const Event& e )
{
    if ( e.cause == Event::AddedChild ) {
	receive_changes_from( static_cast<const LinkChangeEvent&>(e).other );
    }
}

}
