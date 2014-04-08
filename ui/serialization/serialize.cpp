#include "ui/serialization/serialize.h"
#include "ui/serialization/Node.h"
#include <fstream>

namespace simparm {
namespace serialization_ui {

void serialize( const dStorm::JobConfig& orig_config, std::string filename ) {
    std::auto_ptr< dStorm::JobConfig > config( orig_config.clone() );
    std::ofstream target( filename.c_str() );
    boost::shared_ptr<simparm::serialization_ui::Node> n = simparm::serialization_ui::Node::create_root_node();
    config->attach_children_ui( n );
    n->serialize( target );
}

}
}
