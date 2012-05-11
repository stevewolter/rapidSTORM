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
  starter( &master ),
  main_thread(master)
{
    this->showTabbed = true;
    setDesc( makeProgramDescription() );
    if ( display::Manager::getSingleton().getConfig() )
        this->push_back( *display::Manager::getSingleton().getConfig() );
}

InputStream::~InputStream() {}

void InputStream::set_config( const job::Config& config ) {
    orig_config.reset( new job::Config(config) );
    reset_config();
}

void InputStream::reset_config() {
    if ( orig_config.get() ) {
        current_config.reset( new job::Config(*orig_config) );
        current_config->registerNamedEntries( *this );
        current_config->push_back( starter );
        starter.setConfig( *current_config );
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

void InputStream::print(const std::string& what) {
    ost::DebugStream::get()->ost::LockedStream::begin();
    simparm::IO::print(what);
    ost::DebugStream::get()->ost::LockedStream::end();
}


}
