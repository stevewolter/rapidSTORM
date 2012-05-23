#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "test-plugin/cpu_time.h"
#include "MainThread.h"

#include <dStorm/display/Manager.h>
#include <dStorm/helpers/thread.h>

namespace dStorm {

InputStream::InputStream( MainThread& master )
: IO(NULL,NULL),
  main_thread(master),
  desc( "desc", PACKAGE_STRING ),
  viewable( "viewable", true ),
  userLevel( "userLevel", 10 )
{
    this->showTabbed = true;
    this->add_attribute(desc);
    this->add_attribute(viewable);
    this->add_attribute(userLevel);
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
        display::Manager::getSingleton().attach_ui( get_handle() );
        starter.reset( new JobStarter( &main_thread ) );
        starter->setConfig( *current_config );
        starter->attach_ui( current_config->user_interface_handle() );
    }
}

void InputStream::processCommand(const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "wait_for_jobs" ) {
        main_thread.terminate_running_jobs();
    } else if ( cmd == "resource_usage" ) {
        boost::optional<double> cpu_time = get_cpu_time();
        if ( cpu_time )
            std::cout << "Current CPU time: " << *cpu_time << std::endl;
        else
            std::cout << "Resource usage not supported" << std::endl;
    } else if ( cmd == "reset" ) {
        reset_config();
    } else if ( cmd == "quit" ) {
        main_thread.terminate_running_jobs();
        IO::processCommand(cmd,rest);
    } else {
        simparm::IO::processCommand(cmd, rest);
    }
}

bool InputStream::print(const std::string& what) {
    ost::DebugStream::get()->ost::LockedStream::begin();
    bool b = simparm::IO::print(what);
    ost::DebugStream::get()->ost::LockedStream::end();
    return b;
}


}
