#include "JobStarter.h"
#include "job/Car.h"
#include <simparm/Message.h>
#include <simparm/Node.h>
#include <dStorm/GUIThread.h>

namespace dStorm {

JobStarter::JobStarter(simparm::NodeHandle attachment_point, JobConfig& config )
: simparm::TriggerEntry("Run", "Run"),
  config(config),
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

void JobStarter::run_job( boost::shared_ptr<Job> car ) {
    car->run();
    GUIThread::get_singleton().unregister_job( *car );
    car.reset();
    GUIThread::get_singleton().join_this_thread();
}

void JobStarter::start_job() {
    if ( triggered() ) {
      untrigger();
        try {
            boost::shared_ptr< Job > car( config.make_job() );
            simparm::NodeHandle ui = car->attach_ui( attachment_point );
            ui->stop_job_on_ui_detachment( car );
            GUIThread::get_singleton().register_job( *car );
            std::auto_ptr< boost::thread > job_thread
                ( new boost::thread( &JobStarter::run_job, car ) );
            GUIThread::get_singleton().wait_for_thread( job_thread );
        } catch ( const std::runtime_error& e ) {
            simparm::Message m("Starting job failed", e.what() );
            m.send( attachment_point );
        }
    }
}

}
