#ifndef DSTORM_SHELL_JOB_FACTORY_H
#define DSTORM_SHELL_JOB_FACTORY_H

#include <memory>
#include <simparm/NodeHandle.h>
#include "base/Config.h"
#include "shell/JobStarter.h"

namespace dStorm {
namespace shell {

class JobFactory {
    std::auto_ptr< JobConfig > config;
    JobStarter starter;
    simparm::NodeHandle current_ui, config_ui;

    void create_config( boost::optional<std::string> config_file );

public:
    JobFactory( std::auto_ptr< JobConfig >, simparm::NodeHandle );
    void serialize( std::string filename );
    void deserialize( std::string filename );
    void attach_ui( simparm::NodeHandle );
    const JobConfig& current_config() const;
    simparm::NodeHandle config_user_interface_handle() const { return config_ui; }
};

}
}

#endif
