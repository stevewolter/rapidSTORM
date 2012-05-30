#include "Node.h"
#include "TextfieldNode.h"
#include "TabNode.h"
#include "ChoiceNode.h"
#include "TriggerNode.h"
#include "ProgressNode.h"
#include "wxDisplay/wxManager.h"

namespace simparm {
namespace wx_ui {

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

}
}
