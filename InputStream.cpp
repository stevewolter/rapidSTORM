#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "test-plugin/cpu_time.h"
#include "MainThread.h"

#include <dStorm/helpers/thread.h>
#include <simparm/text_stream/BackendRoot.h>

namespace dStorm {

class InputStream::Backend : public simparm::text_stream::BackendRoot {
    virtual void process_command_( const std::string& command, std::istream& rest );
    InputStream& frontend;
    MainThread& main_thread;
public:
    Backend(  InputStream& frontend ) 
        : BackendRoot(&std::cout), frontend(frontend), main_thread(frontend.main_thread) {}
};

InputStream::InputStream( MainThread& master )
: simparm::text_stream::Node("IO", "IO"),
  main_thread(master),
  root_backend( new Backend(*this) )
{
    set_backend_node( std::auto_ptr<simparm::text_stream::BackendNode>(root_backend) );
}

void InputStream::processCommand( std::istream& i ) {
    return root_backend->processCommand( i );
}

InputStream::~InputStream() {}

void InputStream::set_config( const job::Config& config ) {
    orig_config.reset( new job::Config(config) );
    reset_config();
}

void InputStream::reset_config() {
    if ( orig_config.get() ) {
        current_config.reset( new job::Config(*orig_config) );
        current_config->attach_ui( get_handle() );
        starter.reset( new JobStarter( &main_thread ) );
        starter->setConfig( *current_config );
        starter->attach_ui( current_config->user_interface_handle() );
    }
}

void InputStream::Backend::process_command_(const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "terminate_jobs" ) {
        main_thread.terminate_running_jobs();
        std::cout << "Current job count is " << main_thread.count_jobs() << std::endl;
    } else if ( cmd == "job_count" ) {
        std::cout << "Current job count is " << main_thread.count_jobs() << std::endl;
    } else if ( cmd == "resource_usage" ) {
        boost::optional<double> cpu_time = get_cpu_time();
        if ( cpu_time )
            std::cout << "Current CPU time: " << *cpu_time << std::endl;
        else
            std::cout << "Resource usage not supported" << std::endl;
    } else if ( cmd == "reset" ) {
        frontend.reset_config();
    } else if ( cmd == "quit" ) {
        main_thread.terminate_running_jobs();
        BackendRoot::process_command_(cmd,rest);
    } else {
        BackendRoot::process_command_(cmd, rest);
    }
}

bool InputStream::received_quit_command() {
    return root_backend->received_quit_command();
}

}
