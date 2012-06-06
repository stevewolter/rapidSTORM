#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "test-plugin/cpu_time.h"
#include "MainThread.h"

#include <simparm/text_stream/BackendRoot.h>
#include <dStorm/display/Manager.h>

namespace dStorm {

class InputStream::Backend : public simparm::text_stream::BackendRoot {
    virtual void process_command_( const std::string& command, std::istream& rest );
    InputStream& frontend;
    MainThread& main_thread;
public:
    Backend(  InputStream& frontend ) 
        : BackendRoot(&std::cout), frontend(frontend), main_thread(frontend.main_thread) {}
};

InputStream::InputStream( MainThread& master, const job::Config& config )
: simparm::text_stream::Node("IO", "IO"),
  orig_config( new job::Config(config) ),
  main_thread(master),
  root_backend( new Backend(*this) )
{
    main_thread.register_unstopable_job();
    set_backend_node( std::auto_ptr<simparm::text_stream::BackendNode>(root_backend) );
}

void InputStream::processCommands() {
    std::cout << "# rapidSTORM waiting for commands" << std::endl;
    while ( ! received_quit_command() ) {
        int peek = std::cin.peek();
        if ( isspace( peek ) ) {
            std::cin.get();
            continue;
        } else if ( peek == std::char_traits<char>::eof() ) {
            break;
        }
        try {
            root_backend->processCommand( std::cin );
        } catch (const std::bad_alloc& e) {
            std::cerr << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Could not perform action: "
                      << e.what() << std::endl;
        }
    }
}

InputStream::~InputStream() {
    main_thread.unregister_unstopable_job();
}

void InputStream::reset_config() {
    if ( orig_config.get() ) {
        current_config.reset( new job::Config(*orig_config) );
        current_config->attach_ui( get_handle() );
        starter.reset( new JobStarter( &main_thread, get_handle(), *current_config ) );
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
    } else if ( cmd == "serialize" ) {
        std::string target_file;
        while (rest.peek() == ' ' || rest.peek() == '\t') rest.get();
        std::getline( rest, target_file );
        job::serialize( *frontend.current_config, target_file );
    } else {
        BackendRoot::process_command_(cmd, rest);
    }
}

bool InputStream::received_quit_command() {
    return root_backend->received_quit_command();
}

boost::shared_ptr<InputStream> InputStream::create( MainThread& m, const job::Config& c ) {
    boost::shared_ptr<InputStream> rv( new InputStream( m, c ) );
    rv->reset_config();
    rv->root_backend->attach_ui( rv );
    return rv;
}

}
