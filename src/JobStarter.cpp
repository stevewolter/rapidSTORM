#include "JobStarter.h"
#include <simparm/Message.hh>
#include <dStorm/helpers/exception.h>

#include "debug.h"
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
            DEBUG("Running job");
            std::auto_ptr<engine::Car> car( 
                new engine::Car(master, *config) );
            car->detach();
            car.release();
        } catch ( const dStorm::runtime_error& e ) {
            simparm::Message m( e.get_message("Starting job failed") );
            const_cast<engine::CarConfig&>(*config).send( m );
        } catch ( const std::exception& e ) {
            simparm::Message m("Starting job failed", e.what() );
            const_cast<engine::CarConfig&>(*config).send( m );
        }
      }
    }
}

}
