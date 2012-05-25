#ifndef DSTORM_COMMANDLINE_H
#define DSTORM_COMMANDLINE_H

#include "MainThread.h"
#include "JobStarter.h"
#include <dStorm/Config.h>

namespace dStorm {

class CommandLine
{
    job::Config config;
    JobStarter starter;
    MainThread& main_thread;
    simparm::NodeHandle cmdline_ui;

    int find_config_file( int argc, char* argv[] );
    bool load_config_file( const std::string& );

  public:
    CommandLine( MainThread& main_thread );
    ~CommandLine();

    void parse( int argc, char* argv[] );
};

}

#endif
