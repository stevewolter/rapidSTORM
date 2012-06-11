#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "test-plugin/cpu_time.h"
#include "dStorm/GUIThread.h"

#include <simparm/text_stream/BackendRoot.h>
#include <ui/serialization/serialize.h>

namespace dStorm {

class InputStream::Backend : public simparm::text_stream::BackendRoot {
    virtual void process_command_( const std::string& command, std::istream& rest );
    InputStream& frontend;
public:
    Backend(  InputStream& frontend, bool wxWidgets ) 
        : BackendRoot(&std::cout, wxWidgets), frontend(frontend) {}
};

InputStream::InputStream( const JobConfig& config, bool wxWidgets )
: simparm::text_stream::Node("IO", "IO"),
  orig_config( config.clone() ),
  root_backend( new Backend(*this, wxWidgets) )
{
    GUIThread::get_singleton().register_unstopable_job();
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
    current_config.reset();
    starter.reset();
}

InputStream::~InputStream() {
    GUIThread::get_singleton().unregister_unstopable_job();
}

void InputStream::reset_config() {
    if ( orig_config.get() ) {
        current_config.reset( orig_config->clone() );
        simparm::NodeHandle job_ui = current_config->attach_ui( get_handle() );
        starter.reset( new JobStarter( get_handle(), *current_config ) );
        starter->attach_ui( job_ui );
    }
}

void InputStream::Backend::process_command_(const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "terminate_jobs" ) {
        GUIThread::get_singleton().terminate_running_jobs();
        std::cout << "Current job count is " << GUIThread::get_singleton().count_jobs() << std::endl;
    } else if ( cmd == "job_count" ) {
        std::cout << "Current job count is " << GUIThread::get_singleton().count_jobs() << std::endl;
    } else if ( cmd == "resource_usage" ) {
        boost::optional<double> cpu_time = get_cpu_time();
        if ( cpu_time )
            std::cout << "Current CPU time: " << *cpu_time << std::endl;
        else
            std::cout << "Resource usage not supported" << std::endl;
    } else if ( cmd == "reset" ) {
        frontend.reset_config();
    } else if ( cmd == "quit" ) {
        GUIThread::get_singleton().terminate_running_jobs();
        BackendRoot::process_command_(cmd,rest);
    } else if ( cmd == "serialize" ) {
        std::string target_file;
        while (rest.peek() == ' ' || rest.peek() == '\t') rest.get();
        std::getline( rest, target_file );
        simparm::serialization_ui::serialize( *frontend.current_config, target_file );
    } else {
        BackendRoot::process_command_(cmd, rest);
    }
}

bool InputStream::received_quit_command() {
    return root_backend->received_quit_command();
}

boost::shared_ptr<InputStream> InputStream::create( const JobConfig& c, bool wxWidgets ) {
    boost::shared_ptr<InputStream> rv( new InputStream( c, wxWidgets ) );
    rv->reset_config();
    if ( ! wxWidgets )
        rv->root_backend->attach_ui( rv );
    return rv;
}

}
