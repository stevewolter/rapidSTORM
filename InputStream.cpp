#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "test-plugin/cpu_time.h"
#include "GUIThread.h"
#include "alignment_fitter.h"
#include "shell/ReplayJob.h"

#include "simparm/text_stream/BackendRoot.h"
#include "simparm/wx_ui/no_main_window.h"
#include <ui/serialization/serialize.h>

namespace dStorm {

class InputStream::Backend : public simparm::text_stream::BackendRoot {
    virtual void process_command_( const std::string& command, std::istream& rest );
    InputStream& frontend;
public:
    Backend(  InputStream& frontend, bool wxWidgets ) 
        : BackendRoot(&std::cout, wxWidgets ? simparm::wx_ui::no_main_window() : simparm::NodeHandle()), frontend(frontend) {}
    boost::recursive_mutex* get_mutex() { return simparm::text_stream::BackendRoot::get_mutex(); }
};

InputStream::InputStream( const JobConfig& config, bool wxWidgets )
: simparm::text_stream::Node("IO", "IO"),
  rapidstorm( std::auto_ptr<JobConfig>(config.clone()) ),
  alignment_fitter( make_alignment_fitter_config() ),
  replay_job( shell::make_replay_job() ),
  root_backend( new Backend(*this, wxWidgets) )
{
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
            boost::lock_guard< boost::recursive_mutex > m( *root_backend->get_mutex() );
            std::cout << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::runtime_error& e) {
            boost::lock_guard< boost::recursive_mutex > m( *root_backend->get_mutex() );
            std::cout << "Could not perform action: "
                      << e.what() << std::endl;
        }
    }
    configs.clear();
}

InputStream::~InputStream() {
}

void InputStream::reset_config() {
    configs.clear();
}

void InputStream::create_localization_job() {
    configs.push_back( rapidstorm.create_config( get_handle() ) );
}

void InputStream::create_alignment_fitter() {
    configs.push_back( alignment_fitter.create_config( get_handle() ) );
}

void InputStream::create_replay_job() {
    configs.push_back( replay_job.create_config( get_handle() ) );
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
    } else if ( cmd == "localization" ) {
        frontend.create_localization_job();
    } else if ( cmd == "alignment_fitter" ) {
        frontend.create_alignment_fitter();
    } else if ( cmd == "replay_job" ) {
        frontend.create_replay_job();
    } else if ( cmd == "quit" ) {
        GUIThread::get_singleton().terminate_running_jobs();
        BackendRoot::process_command_(cmd,rest);
    } else if ( cmd == "serialize" ) {
        std::string target_file;
        while (rest.peek() == ' ' || rest.peek() == '\t') rest.get();
        std::getline( rest, target_file );
        frontend.configs.front().serialize( target_file );
    } else {
        BackendRoot::process_command_(cmd, rest);
    }
}

bool InputStream::received_quit_command() {
    return root_backend->received_quit_command();
}

boost::shared_ptr<InputStream> InputStream::create( const JobConfig& c, bool wxWidgets ) {
    boost::shared_ptr<InputStream> rv( new InputStream( c, wxWidgets ) );
    rv->create_localization_job();
    if ( ! wxWidgets )
        rv->root_backend->attach_ui( rv );
    return rv;
}

}
