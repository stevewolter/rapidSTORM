#include "debug.h"

#include "JobStarter.h"
#include <simparm/Message.hh>

namespace dStorm {

JobStarter::JobStarter(JobMaster* m )
: simparm::TriggerEntry("Run", "Run"),
  simparm::Listener(simparm::Event::ValueChanged),
  master(m), config(NULL)
{
    setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    setUserLevel(simparm::Object::Beginner);

    receive_changes_from( value );
}

void JobStarter::operator()( const simparm::Event& ) {
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
