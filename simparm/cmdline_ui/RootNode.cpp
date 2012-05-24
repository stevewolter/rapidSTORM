#include "RootNode.h"
#include "OptionTable.h"

namespace simparm {
namespace cmdline_ui {

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

}
}
