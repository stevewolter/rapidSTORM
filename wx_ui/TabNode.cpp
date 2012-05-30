#include "TabNode.h"
#include "WindowNode.h"
#include <wx/notebook.h>
#include "wxDisplay/wxManager.h"

namespace simparm {
namespace wx_ui {

static void make_notebook( 
    boost::shared_ptr<wxNotebook*> notebook,
    boost::shared_ptr< wxWindow* > window_view,
    boost::shared_ptr< wxWindow* > parent_window
) {
    *notebook = new wxNotebook( *parent_window, wxID_ANY );
    *window_view = *notebook;
}

void TabNode::initialization_finished() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_notebook, notebook, window.window, Node::get_parent_window() ) );
    Node::add_full_width_line( window );
}

static void add_tab(
    boost::shared_ptr<wxNotebook*> notebook,
    const WindowSpecification& w
) {
    (*notebook)->AddPage( *w.window, wxString( w.name.c_str(), wxConvUTF8 ) );
}

void TabNode::add_entry_line( const LineSpecification& ) { throw std::logic_error("Entry cannot be a direct child of tab group"); }
void TabNode::add_full_width_line( WindowSpecification w ) {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &add_tab, notebook, w ) );
}

NodeHandle TabNode::create_object( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

NodeHandle TabNode::create_group( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

}
}
