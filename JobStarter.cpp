#include "debug.h"

#include "JobStarter.h"
#include <simparm/Message.hh>

namespace dStorm {

JobStarter::JobStarter(JobMaster* m )
: simparm::TriggerEntry("Run", "Run"),
  master(m), config(NULL)
{
    setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    setUserLevel(simparm::Object::Beginner);
}

void JobStarter::attach_ui( simparm::Node& n ) {
    simparm::TriggerEntry::attach_ui(n);
    listening = value.notify_on_value_change( boost::bind( &JobStarter::start_job, this ) );
}

void JobStarter::start_job() {
    if ( triggered() ) {
      untrigger();
      if ( config != NULL ) {
        try {
            DEBUG("Creating job");
            config->create_and_run( *master );
            DEBUG("Running job");
        } catch ( const std::runtime_error& e ) {
            simparm::Message m("Starting job failed", e.what() );
            DEBUG("Got normal exception");
            const_cast<job::Config&>(*config).send( m );
        }
        DEBUG("Finished handling job start");
      }
    }
}

}
