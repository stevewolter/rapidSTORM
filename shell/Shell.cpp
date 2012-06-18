#include "JobMetaFactory.h"
#include <ui/serialization/serialize.h>
#include <ui/serialization/deserialize.h>
#include <simparm/NodeHandle.h>

namespace dStorm {
namespace shell {

JobMetaFactory::JobMetaFactory( std::auto_ptr< dStorm::JobConfig > j )
: original_config(j) {}

std::auto_ptr<JobFactory> JobMetaFactory::create_config( simparm::NodeHandle user_interface ) const {
    std::auto_ptr<JobFactory> rv( new JobFactory( std::auto_ptr<JobConfig>(original_config->clone()), user_interface ) );
    rv->attach_ui( user_interface );
    return rv;
}

std::auto_ptr<JobFactory> JobMetaFactory::create_config( std::string config_file, simparm::NodeHandle user_interface ) const {
    std::auto_ptr<JobFactory> rv( new JobFactory( std::auto_ptr<JobConfig>(original_config->clone()), user_interface ) );
    rv->deserialize( config_file );
    rv->attach_ui( user_interface );
    return rv;
}

JobFactory::JobFactory( std::auto_ptr< JobConfig > job, simparm::NodeHandle user_interface )
: config( job ),
  starter( user_interface, *config ),
  current_ui( user_interface )
{
}

void JobFactory::serialize( std::string filename ) {
    assert( config.get() );
    simparm::serialization_ui::serialize( *config, filename );
}

void JobFactory::deserialize( std::string filename ) {
    simparm::serialization_ui::deserialize( *config, filename, current_ui );
}

void JobFactory::attach_ui( simparm::NodeHandle user_interface ) {
    config_ui = config->attach_ui( user_interface );
    starter.attach_ui( config_ui );
}

}
}

