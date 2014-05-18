#include "ui/serialization/deserialize.h"
#include "simparm/text_stream/RootNode.h"
#include <fstream>
#include <boost/smart_ptr/make_shared.hpp>

namespace simparm {
namespace serialization_ui {

bool deserialize( dStorm::JobConfig& config, std::string name, simparm::NodeHandle current_ui ) {
    std::ifstream config_file( name.c_str() );
    boost::shared_ptr< simparm::text_stream::RootNode > ui 
        = boost::make_shared< simparm::text_stream::RootNode >();
    config.attach_children_ui( ui );
    if ( !config_file )
        return false;
    else {
        try {
            while ( config_file ) {
                while ( config_file && std::isspace( config_file.peek() ) )
                    config_file.get();
                if ( ! config_file || config_file.peek() == EOF ) break;
                ui->processCommand( config_file );
            }
        } catch (const std::runtime_error& e) {
            simparm::Message m( "Unable to read initialization file " + name, e.what() );
            m.send( current_ui );
        }
        return true;
    }
}

}
}
