#include "debug.h"

#include "JobStarter.h"
#include <simparm/Message.hh>
#include <dStorm/helpers/exception.h>

#include "engine/Car.h"

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
            new engine::Car(master, *config);
            DEBUG("Running job");
        } catch ( const dStorm::runtime_error& e ) {
            simparm::Message m( e.get_message("Starting job failed") );
            DEBUG("Got dStorm exception");
            const_cast<Config&>(*config).send( m );
        } catch ( const std::exception& e ) {
            simparm::Message m("Starting job failed", e.what() );
            DEBUG("Got normal exception");
            const_cast<Config&>(*config).send( m );
        } catch (...) {
            simparm::Message m("Starting job failed", "Starting the job failed for an unknown reason" );
            DEBUG("Got unknown exception");
            const_cast<Config&>(*config).send( m );
        }
        DEBUG("Finished handling job start");
      }
    }
}

}
