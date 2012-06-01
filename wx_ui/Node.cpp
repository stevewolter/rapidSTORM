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
#include "wxDisplay/wxManager.h"
#include <boost/lambda/construct.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <wx/defs.h>
#include <wx/string.h>
#include <wx/stattext.h>
#include "VisibilityControl.h"

namespace simparm {
namespace wx_ui {

namespace bl = boost::lambda;

std::auto_ptr<dStorm::display::WindowHandle> Node::get_image_window( 
    const dStorm::display::WindowProperties& wp, dStorm::display::DataSource& ds )
{
    return dStorm::display::wxManager::get_singleton_instance().register_data_source( wp, ds );
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
    return NodeHandle( new TextfieldNode(shared_from_this()) ); 
}

NodeHandle Node::create_group( std::string name ) { 
    return NodeHandle( new GroupNode(shared_from_this()) ); 
}

NodeHandle Node::create_object( std::string name ) { 
    return NodeHandle( new Node(shared_from_this()) ); 
}

NodeHandle Node::create_tree_root() {
    return NodeHandle( new TreeRoot(shared_from_this()) ); 
}
NodeHandle Node::create_tree_object( std::string ) {
    boost::shared_ptr< Node > tree_page( new TreePage(shared_from_this()) );
    return NodeHandle( new WindowNode( tree_page ) );
}

void Node::run_in_GUI_thread( boost::function0<void> f ) {
    dStorm::display::wxManager& m = dStorm::display::wxManager::get_singleton_instance();
    m.run_in_GUI_thread( f );
}

void Node::create_static_text( boost::shared_ptr<wxWindow*> into, std::string text ) {
    run_in_GUI_thread( *bl::constant( into ) = 
            bl::bind( bl::new_ptr< wxStaticText >(), *bl::constant(get_parent_window()), int(wxID_ANY), 
                                                     wxString( text.c_str(), wxConvUTF8 ) ) );
}

void Node::add_to_visibility_control( boost::shared_ptr<wxWindow*> w ) {
    run_in_GUI_thread( boost::bind( &VisibilityControl::register_window, 
        get_visibility_control(), w, user_level, is_visible ) );
}

}
}
