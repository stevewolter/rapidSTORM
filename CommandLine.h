#ifndef DSTORM_COMMANDLINE_H
#define DSTORM_COMMANDLINE_H

#include <dStorm/Config.h>
#include "job/Config.h"

namespace dStorm {

class CommandLine
{
    job::Config config;
    simparm::NodeHandle cmdline_ui;

    int find_config_file( int argc, char* argv[] );
    bool load_config_file( const std::string& );
    void run_unit_tests( char* arg0, int& exit_status );

  public:
    CommandLine();
    ~CommandLine();

    int parse( int argc, char* argv[] );
};

}

#endif
