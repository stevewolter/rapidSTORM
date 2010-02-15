#include "JobStarter.h"

#include "debug.h"
#include "engine/Car.h"

namespace dStorm {

JobStarter::JobStarter( const engine::CarConfig& c, JobMaster& m )
: simparm::TriggerEntry("Run", "Run"),
  simparm::Listener(simparm::Event::ValueChanged),
  master(m), config(c)
{
    setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    setUserLevel(simparm::Entry::Beginner);

    receive_changes_from( value );
}

void JobStarter::operator()( const simparm::Event& ) {
    if ( triggered() ) {
        untrigger();
        try {
            DEBUG("Running job");
            std::auto_ptr<engine::Car> car( 
                new engine::Car(master, config) );
            car->detach();
            car.release();
        } catch ( const std::exception& e ) {
            std::cerr << "Starting job failed: " << e.what() << "\n";
        }
    }
}

}
