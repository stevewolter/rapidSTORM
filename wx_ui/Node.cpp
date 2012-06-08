#include "Node.h"
#include "TextfieldNode.h"
#include "TabNode.h"
#include "ChoiceNode.h"
#include "TriggerNode.h"
#include "ProgressNode.h"
#include "CheckboxNode.h"
#include "GroupNode.h"
#include "TreeRoot.h"
#include "TreePage.h"
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <wx/defs.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include "VisibilityControl.h"
#include "image_window/create.h"
#include "gui_thread.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

WindowSpecification::WindowSpecification()
: window( new Window() ), proportion(0)
{}

WindowSpecification::~WindowSpecification() {
    std::for_each( removal_instructions.begin(), removal_instructions.end(),
        &run_in_GUI_thread );
}

LineSpecification::LineSpecification( boost::function0<void> redraw_function )
: label( new Window() ),
  contents( new Window() ),
  adornment( new Window() )
{
    label->set_redraw_function( redraw_function );
    contents->set_redraw_function( redraw_function );
    adornment->set_redraw_function( redraw_function );
}

std::auto_ptr<dStorm::display::WindowHandle> Node::get_image_window( 
    const dStorm::display::WindowProperties& wp, dStorm::display::DataSource& ds )
{
    return image_window::create( wp, ds );
}

simparm::NodeHandle Node::create_textfield( std::string, std::string ) {
    return NodeHandle( new TextfieldNode( shared_from_this() ) );
}

simparm::NodeHandle Node::create_tab_group( std::string ) {
    return NodeHandle( new TabNode( shared_from_this() ) );
}

simparm::NodeHandle Node::create_choice( std::string ) {
    return NodeHandle( new ChoiceNode( shared_from_this() ) );
}

NodeHandle Node::create_trigger( std::string name ) { 
    return NodeHandle( new TriggerNode(shared_from_this()) ); 
}

NodeHandle Node::create_progress_bar( std::string name ) { 
    return NodeHandle( new ProgressNode(shared_from_this()) ); 
}

NodeHandle Node::create_checkbox( std::string name ) { 
    return NodeHandle( new CheckboxNode(shared_from_this()) ); 
}

NodeHandle Node::create_file_entry( std::string name ) { 
    return NodeHandle( new TextfieldNode(shared_from_this(), TextfieldNode::File) ); 
}

NodeHandle Node::create_group( std::string name ) { 
    return NodeHandle( new GroupNode(shared_from_this()) ); 
}

NodeHandle Node::create_object( std::string name ) { 
    return NodeHandle( new InnerNode(shared_from_this()) ); 
}

NodeHandle Node::create_tree_root() {
    return NodeHandle( new TreeRoot(shared_from_this()) ); 
}
NodeHandle Node::create_tree_object( std::string ) {
    boost::shared_ptr< InnerNode > tree_page( new TreePage(shared_from_this()) );
    return NodeHandle( new WindowNode( tree_page ) );
}

void InnerNode::create_static_text( boost::shared_ptr<Window> into, std::string text ) {
    run_in_GUI_thread( *bl::constant( into ) = 
            bl::bind( bl::new_ptr< wxStaticText >(), *bl::constant(get_parent_window()), int(wxID_ANY), 
                                                     wxString( text.c_str(), wxConvUTF8 ) ) );
}

void InnerNode::attach_help( boost::shared_ptr<Window> to ) {
    attach_context_help( to, help_id );
    void (wxWindow::* set_tool_tip)(const wxString&) = &wxWindow::SetToolTip;
    if ( help_message != "" )
        run_in_GUI_thread( 
            bl::bind( set_tool_tip, * bl::constant(to), wxString( help_message.c_str(), wxConvUTF8 ) ) );
}

}
}
