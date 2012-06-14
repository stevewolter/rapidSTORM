#include "ScrolledWindowNode.h"
#include <ui/serialization/serialize.h>
#include <ui/serialization/deserialize.h>
#include "MainConfig.h"
#include "Node.h"

namespace simparm {
namespace wx_ui {

ConfigTemplate::ConfigTemplate( std::auto_ptr< dStorm::JobConfig > j, NodeHandle n )
: original_config(j), user_interface(n) {}

std::auto_ptr<MainConfig> ConfigTemplate::create_config() {
    std::auto_ptr<MainConfig> rv( new MainConfig( *original_config, user_interface ) );
    rv->attach_ui( user_interface );
    return rv;
}

std::auto_ptr<MainConfig> ConfigTemplate::create_config( std::string config_file ) {
    std::auto_ptr<MainConfig> rv( new MainConfig( *original_config, user_interface ) );
    rv->deserialize( config_file );
    rv->attach_ui( user_interface );
    return rv;
}

MainConfig::MainConfig( const dStorm::JobConfig& job, NodeHandle user_interface )
: config( job.clone() ),
  starter( user_interface, *config ),
  current_ui( user_interface )
{
}

void MainConfig::serialize( std::string filename ) {
    assert( config.get() );
    simparm::serialization_ui::serialize( *config, filename );
}

void MainConfig::deserialize( std::string filename ) {
    serialization_ui::deserialize( *config, filename, current_ui );
}

void MainConfig::attach_ui( NodeHandle user_interface ) {
    config_ui = config->attach_ui( user_interface );
    starter.attach_ui( config_ui );
}

void MainConfig::ui_managed( std::auto_ptr<MainConfig> p ) {
    ScrolledWindowNode* sw = dynamic_cast< ScrolledWindowNode* >( p->config_ui.get() );
    assert( sw );
    sw->set_config( p );
}

}
}
