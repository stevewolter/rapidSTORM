#ifndef DSTORM_SHELL_H
#define DSTORM_SHELL_H

#include <string>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/Config.h>
#include "shell/JobFactory.h"

namespace dStorm {
namespace shell {

class JobMetaFactory {
    boost::shared_ptr< JobConfig > original_config;
public:
    JobMetaFactory( std::auto_ptr< JobConfig > );
    std::auto_ptr<JobFactory> create_config( simparm::NodeHandle ) const;
    std::auto_ptr<JobFactory> create_config( std::string config_file, simparm::NodeHandle ) const;
};

}
}

#endif
