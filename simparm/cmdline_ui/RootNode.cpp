#include "simparm/cmdline_ui/RootNode.h"
#include "simparm/cmdline_ui/OptionTable.h"
#include "simparm/wx_ui/no_main_window.h"

namespace simparm {
namespace cmdline_ui {

RootNode::RootNode()
: Node("Root")
{
}

simparm::NodeHandle RootNode::create_group( std::string name ) {
    return Node::create_group( name );
}

void RootNode::print_help() {
    OptionTable table;
    program_options( table );
    table.printHelp( std::cerr );
}

void RootNode::parse_command_line( int argc, char **argv ) {
    int co = 1;
    while ( co < argc ) {
        OptionTable table;
        program_options( table );
        int parsed = table.parse( argc-co, argv+co );
        if ( parsed == 0 ) {
            std::cerr << "Ignored unknown argument " << argv[co] << std::endl;
            ++co;
        } else
            co += parsed;
    }
}

Message::Response RootNode::send( Message& m ) const {
    std::cerr << m;
    return Message::OKYes;
}

std::auto_ptr<dStorm::display::WindowHandle> RootNode::get_image_window( 
    const dStorm::display::WindowProperties& ds, dStorm::display::DataSource& wp )
{
    if ( ! wx_ui_window_creator )
        wx_ui_window_creator = wx_ui::no_main_window();
    return wx_ui_window_creator->get_image_window( ds, wp );
}

}
}
