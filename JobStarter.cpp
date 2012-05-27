#include "debug.h"

#include "JobStarter.h"
#include "job/Car.h"
#include <simparm/Message.h>

namespace dStorm {

JobStarter::JobStarter(MainThread* m, simparm::NodeHandle attachment_point, job::Config& config )
: simparm::TriggerEntry("Run", "Run"),
  master(m), config(config),
  attachment_point( attachment_point )
{
    setHelp("Whenever this trigger is triggered or the button "
                "clicked, the dStorm engine will be run with the "
                "current parameters.");
    set_user_level(simparm::Beginner);
}

void JobStarter::attach_ui( simparm::NodeHandle n ) {
    simparm::TriggerEntry::attach_ui(n);
    listening = value.notify_on_value_change( boost::bind( &JobStarter::start_job, this ) );
}

void JobStarter::run_job( boost::shared_ptr<job::Car> car, MainThread* master ) {
    DEBUG("Running job");
    car->drive_exception_safe();
    master->unregister_job( *car );
    DEBUG("Finished job");
}

void JobStarter::start_job() {
    if ( triggered() ) {
      untrigger();
        try {
            DEBUG("Creating job");
            boost::shared_ptr< job::Car > car( new job::Car(config) );
            car->attach_ui( attachment_point );
            master->register_job( *car );
            boost::thread job_thread( &JobStarter::run_job, car, master );
            job_thread.detach();
            DEBUG("Job moved to background");
        } catch ( const std::runtime_error& e ) {
            simparm::Message m("Starting job failed", e.what() );
            DEBUG("Got normal exception");
            m.send( attachment_point );
        }
        DEBUG("Finished handling job start");
    }
}

}
