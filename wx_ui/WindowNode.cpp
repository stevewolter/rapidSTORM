#include "WindowNode.h"
#include <wx/wx.h>
#include "wxDisplay/wxManager.h"
#include <wx/gbsizer.h>
#include "lambda.h"

namespace simparm {
namespace wx_ui {

WindowNode::WindowNode( boost::shared_ptr<Node> n ) 
: Node(n),
  sizer( new wxGridBagSizer*(NULL) ),
  row( new int(0) )
{
}

static void make_window( 
    boost::shared_ptr< wxWindow* > window_view,
    boost::shared_ptr< wxWindow* > parent_window
) {
    *window_view = new wxPanel( *parent_window, wxID_ANY, wxDefaultPosition, wxDefaultSize );
}

static void create_sizer( boost::shared_ptr<wxGridBagSizer*> sizer ) {
    *sizer = new wxGridBagSizer();
    (*sizer)->SetEmptyCellSize( wxSize(0,0) );
    (*sizer)->AddGrowableCol(1);
}

void WindowNode::create_unattached_gridbag_sizer() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &wx_ui::create_sizer, sizer ) );
}

void WindowNode::create_sizer() {
    create_unattached_gridbag_sizer();
    run_in_GUI_thread( bl::bind( &wxWindow::SetSizer, *bl::constant(window.window), *bl::constant( sizer ), true ) );
}

void WindowNode::initialization_finished() {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &make_window, window.window, Node::get_parent_window() ) );
    create_sizer();
    Node::add_full_width_line( window );
}

static void add_entry_line( 
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<int> row,
    const LineSpecification& line
) {
    if ( *line.label )
        (*sizer)->Add( *line.label, wxGBPosition(*row,0) );
    if ( *line.adornment ) {
        (*sizer)->Add( *line.contents, wxGBPosition(*row,1), wxGBSpan(), wxGROW );
        (*sizer)->Add( *line.adornment, wxGBPosition(*row,2), wxGBSpan(), wxALIGN_CENTER );
    } else {
        (*sizer)->Add( *line.contents, wxGBPosition(*row,1), wxGBSpan(), wxGROW );
    }
    (*sizer)->Layout();
    ++ *row;
}

static void add_full_width_line(
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<int> row,
    const WindowSpecification& w
) {
    (*sizer)->Add( *w.window, wxGBPosition(*row,0), wxGBSpan(1,3), wxGROW | wxEXPAND );
    (*sizer)->Layout();
    ++ *row;
}

static void add_full_width_sizer(
    boost::shared_ptr<wxGridBagSizer*> sizer, 
    boost::shared_ptr<int> row,
    const SizerSpecification& w
) {
    (*sizer)->Add( *w.sizer, wxGBPosition(*row,0), wxGBSpan(1,3), wxGROW | wxEXPAND );
    (*sizer)->Layout();
    ++ *row;
}

void WindowNode::add_entry_line( const LineSpecification& line ) {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &wx_ui::add_entry_line, sizer, row, line ) );
}

void WindowNode::add_full_width_line( WindowSpecification w ) {
    dStorm::display::wxManager::get_singleton_instance().run_in_GUI_thread(
        boost::bind( &wx_ui::add_full_width_line, sizer, row, w ) );
}

void WindowNode::add_full_width_sizer( SizerSpecification w ) {
    run_in_GUI_thread( boost::bind( &wx_ui::add_full_width_sizer, sizer, row, w ) );
}


}
}
