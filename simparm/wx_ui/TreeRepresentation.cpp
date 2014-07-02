/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/treebook.h>

#include "simparm/wx_ui/TreeRepresentation.h"
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

namespace simparm {
namespace wx_ui {

namespace br = boost::range;

class Treebook : public wxTreebook {
    boost::shared_ptr<bool> valid_;
  public:
    Treebook(wxWindow* parent, boost::shared_ptr<bool> valid) : wxTreebook(parent, wxID_ANY), valid_(valid) {}
    ~Treebook() { *valid_ = false; }
};

TreeRepresentation::TreeRepresentation() : widget(NULL) {}

void TreeRepresentation::add_as_child( 
    boost::shared_ptr< TreeRepresentation > parent, 
    boost::shared_ptr<Window> window,
    wxString name,
    boost::function0<void> redraw
)
{
    assert( parent );
    this->parent = parent;
    parent->children.push_back( this );
    widget_valid = parent->widget_valid;
    widget = parent->widget;
    if ( parent->parent ) {
        int index = parent->get_preceding_and_self_count() - 1;
        widget->InsertSubPage( index, *window, name );
        widget->ExpandNode( index );
    } else {
        widget->AddPage( *window, name );
    }
    widget->InvalidateBestSize();
    redraw();
}

void TreeRepresentation::create_widget(boost::shared_ptr<Window> window_announcement, boost::shared_ptr<Window> parent ) {
    assert( ! this->parent );
    widget_valid.reset(new bool(true));
    *window_announcement = widget = new Treebook( *parent, widget_valid );
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

void TreeRepresentation::remove_child( boost::shared_ptr<Window> ) {
    int index = get_preceding_and_self_count() - 1;
    if (*widget_valid) {
        widget->DeletePage( index );
    }
    boost::range::remove_erase( parent->children, this );
}

}
}
