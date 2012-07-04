/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/notebook.h>

#include "TabNode.h"
#include "WindowNode.h"
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include "lambda.h"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

class Notebook : public wxNotebook {
public:
    Notebook( boost::shared_ptr<Window> parent )
        : wxNotebook( *parent, wxID_ANY ) {}
    void delete_page( boost::shared_ptr<Window> window ) {
        for (size_t i = 0; i < GetPageCount(); ++i) 
            if ( GetPage(i) == *window ) {
                DeletePage(i);
                return;
            }
    }
};

void TabNode::initialization_finished() {
    run_in_GUI_thread(
        *bl::constant( window.window ) =
        *bl::constant( notebook ) =
        bl::bind( bl::new_ptr<Notebook>(), InnerNode::get_parent_window() ) );
    InnerNode::add_full_width_line( window );
}

static void add_tab(
    boost::shared_ptr<Notebook*> notebook,
    boost::shared_ptr<Window> window,
    wxString name
) {
    bool success = (*notebook)->AddPage( *window, name, true );
    assert( success );
}

void TabNode::add_entry_line( LineSpecification& ) { throw std::logic_error("Entry cannot be a direct child of tab group"); }
void TabNode::add_full_width_line( WindowSpecification& w ) {
    run_in_GUI_thread( boost::bind( &add_tab, notebook, w.window, wxString( w.name.c_str(), wxConvUTF8 ) ) );
    w.removal_instructions.push_back( 
        bl::bind( &Notebook::delete_page, *bl::constant(notebook), w.window ) );
}
void TabNode::add_full_width_sizer( SizerSpecification& w ) {
    throw std::logic_error("Sizer cannot be a direct child of tab group");
}

NodeHandle TabNode::create_object( std::string name ) {
    return NodeHandle( new WindowNode( shared_from_this(), name ) );
}

NodeHandle TabNode::create_group( std::string name ) {
    return NodeHandle( new WindowNode( shared_from_this(), name ) );
}

boost::function0<void> TabNode::get_relayout_function() {
    return boost::function0<void>( ( 
        bl::bind( &wxNotebook::InvalidateBestSize, *bl::constant( notebook ) ),
        bl::bind( get_parent_relayout_function() )
    ) );
}

}
}
