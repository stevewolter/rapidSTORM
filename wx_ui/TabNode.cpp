#include "TabNode.h"
#include "WindowNode.h"
#include <wx/notebook.h>
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

void TabNode::initialization_finished() {
    run_in_GUI_thread(
        *bl::constant( window.window ) =
        *bl::constant( notebook ) =
        bl::bind( bl::new_ptr<wxNotebook>(), 
                  *bl::constant(Node::get_parent_window()),
                  int(wxID_ANY) ) );
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
    run_in_GUI_thread( boost::bind( &add_tab, notebook, w ) );
}

NodeHandle TabNode::create_object( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

NodeHandle TabNode::create_group( std::string ) {
    return NodeHandle( new WindowNode( shared_from_this() ) );
}

boost::function0<void> TabNode::get_relayout_function() {
    return boost::function0<void>( ( 
        bl::bind( &wxNotebook::InvalidateBestSize, *bl::constant( notebook ) ),
        bl::bind( Node::get_relayout_function() )
    ) );
}

}
}
