#ifndef SIMPARM_WX_UI_MAINCONFIG_H
#define SIMPARM_WX_UI_MAINCONFIG_H

#include <memory>
#include <string>
#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/Config.h>
#include "JobStarter.h"

namespace simparm {
namespace wx_ui {

class ConfigTemplate;

class MainConfig {
    std::auto_ptr< dStorm::JobConfig > config;
    dStorm::JobStarter starter;
    NodeHandle current_ui, config_ui;

    void create_config( boost::optional<std::string> config_file );

public:
    MainConfig( const dStorm::JobConfig& job, NodeHandle );
    void serialize( std::string filename );
    void deserialize( std::string filename );
    void attach_ui( NodeHandle );
    static void ui_managed( std::auto_ptr<MainConfig> );
};

class ConfigTemplate {
    std::auto_ptr< dStorm::JobConfig > original_config;
    NodeHandle user_interface;
public:
    ConfigTemplate( std::auto_ptr< dStorm::JobConfig >, NodeHandle );
    std::auto_ptr<MainConfig> create_config();
    std::auto_ptr<MainConfig> create_config( std::string config_file );
};

}
}

#endif
