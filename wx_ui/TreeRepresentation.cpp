#include "TreeRepresentation.h"
#include <wx/treebook.h>
#include <boost/range/algorithm/find.hpp>

namespace simparm {
namespace wx_ui {

namespace br = boost::range;

TreeRepresentation::TreeRepresentation() : widget(NULL) {}

void TreeRepresentation::add_as_child( 
    boost::shared_ptr< TreeRepresentation > parent, 
    const WindowSpecification& page ,
    boost::function0<void> redraw
)
{
    assert( parent );
    this->parent = parent;
    parent->children.push_back( this );
    widget = parent->widget;
    wxString name( page.name.c_str(), wxConvUTF8 );
    if ( parent->parent ) {
        int index = parent->get_preceding_and_self_count() - 1;
        widget->InsertSubPage( index, *page.window, name );
        widget->ExpandNode( index );
    } else {
        widget->AddPage( *page.window, name );
    }
    widget->InvalidateBestSize();
    redraw();
}

void TreeRepresentation::create_widget(boost::shared_ptr<wxWindow*> window_announcement, boost::shared_ptr<wxWindow*> parent ) {
    assert( ! this->parent );
    *window_announcement = widget = new wxTreebook( *parent, wxID_ANY );
}

int TreeRepresentation::get_preceding_and_self_count() const {
    if ( ! parent ) return 0;
    int base = parent->get_preceding_and_self_count();
    
    for ( std::vector< TreeRepresentation* >::const_iterator i = parent->children.begin(), e = br::find( parent->children, this ); i != e; ++ i )
        base += (*i)->node_count();
    return base + 1;
}

int TreeRepresentation::node_count() const {
    int rv = 1;
    for ( std::vector< TreeRepresentation* >::const_iterator i = children.begin(); i != children.end(); ++i )
        rv += (*i)->node_count();
    return rv;
}

}
}
