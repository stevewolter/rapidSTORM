/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/aui/auibook.h>

#include "ScrolledTabNode.h"
#include "WindowNode.h"
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "lambda.h"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class AUINotebook : public wxAuiNotebook {
public:
    AUINotebook( boost::shared_ptr<Window> parent )
        : wxAuiNotebook( *parent, wxID_ANY ) {}
    void delete_page( boost::shared_ptr<Window> window ) {
        for (size_t i = 0; i < GetPageCount(); ++i) 
            if ( GetPage(i) == *window ) {
                DeletePage(i);
                return;
            }
    }
    void delete_all_pages() {
        while ( GetPageCount() > 0u ) {
            DeletePage( GetPageCount() - 1 );
        }
    }
};

void ScrolledTabNode::initialization_finished() {
    run_in_GUI_thread(
        *bl::constant( window.window ) =
        *bl::constant( notebook ) =
        bl::bind( bl::new_ptr<AUINotebook>(), InnerNode::get_parent_window() ) );
    InnerNode::add_full_width_line( window );
}

static void add_tab(
    boost::shared_ptr<AUINotebook*> notebook,
    boost::shared_ptr<Window> window,
    wxString name
) {
    bool success = (*notebook)->AddPage( *window, name, true );
    if (!success) {
        throw std::logic_error("Unable to add a page to the notebook");
    }
}

void ScrolledTabNode::add_entry_line( LineSpecification& ) { throw std::logic_error("Entry cannot be a direct child of tab group"); }
void ScrolledTabNode::add_full_width_line( WindowSpecification& w ) {
    run_in_GUI_thread( boost::bind( &add_tab, notebook, w.window, wxString( w.name.c_str(), wxConvUTF8 ) ) );
    w.removal_instructions.push_back( 
        bl::bind( &AUINotebook::delete_page, *bl::constant(notebook), w.window ) );
}
void ScrolledTabNode::add_full_width_sizer( SizerSpecification& w ) {
    throw std::logic_error("Sizer cannot be a direct child of tab group");
}

NodeHandle ScrolledTabNode::create_object( std::string name ) {
    return NodeHandle( new ScrolledWindowNode( shared_from_this(), name ) );
}

NodeHandle ScrolledTabNode::create_group( std::string name ) {
    return NodeHandle( new ScrolledWindowNode( shared_from_this(), name ) );
}

void ScrolledTabNode::serialize_current_tab( std::string filename ) {
    wxWindow* w = (*notebook)->GetPage( (*notebook)->GetSelection() );
    dynamic_cast<ScrolledWindow&>(*w).serialize( filename );
}

void ScrolledTabNode::close_all_tabs() {
    run_in_GUI_thread( bl::bind( &AUINotebook::delete_all_pages, *bl::constant( notebook ) ) );
}

}
}
