#ifndef SIMPARM_CMDLINE_UI_ROOTNODE_H
#define SIMPARM_CMDLINE_UI_ROOTNODE_H

#include "simparm/cmdline_ui/Node.h"

namespace simparm {
namespace cmdline_ui {

class RootNode : public Node {
    NodeHandle wx_ui_window_creator;
public:
    RootNode();
    void parse_command_line( int argc, char **argv );
    Message::Response send( Message& m ) const;
    void print_help();
    std::auto_ptr<dStorm::display::WindowHandle> get_image_window( 
        const dStorm::display::WindowProperties&, dStorm::display::DataSource& );
    simparm::NodeHandle create_group( std::string name );
};

}
}

#endif
