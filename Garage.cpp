#include "Garage.h"
#include "GarageConfig.h"
#include <memory>
#include <algorithm>
#include <functional>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#include <simparm/ChoiceEntry_Impl.hh>

#include "debug.h"

using namespace dStorm;
using namespace dStorm::input;
using namespace dStorm::output;
using namespace std;

char *get_dir_name(char *file) throw();

static bool load_file(const std::string& name, simparm::Node& node) {
    std::ifstream config_file( name.c_str() );
    if ( !config_file )
        return false;
    else {
        while ( config_file ) {
            DEBUG("Processing command from " << name.c_str());
            try {
                node.processCommand( config_file );
            } catch (const std::exception& e) {
                std::cerr << "Error in initialization config file: "
                          << e.what() << "\n";
            }
        }
        return true;
    }
}

Garage::Garage( int argc, char *argv[] ) 
: master( MasterConfig::create() ),
  autoConfig( new GarageConfig(master.get()) ),
  config( *autoConfig )
{
    DEBUG("Garage argument constructor called");

    std::auto_ptr<simparm::Set> config_set_ptr
        = config.make_set();

    simparm::Set& config = *config_set_ptr;

    DEBUG("Checking for relevant environment variables");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    DEBUG("Checking for command line config file");
    if ( !have_file && argc > 3 && string(argv[1]) == "--config" ) {
        have_file = load_file(argv[2], config);
        if ( have_file ) {
            argc -= 2;
            argv += 2;
        }
    }
    DEBUG("Checking for home directory config file");
    if ( !have_file && home != NULL )
        have_file = load_file( std::string(home) + "/.dstorm", config );
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = load_file( 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt",
            config );

    DEBUG("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = 
            config.readConfig(argc, argv);
        if ( first_nonoption == -1 )
            exit(1);
    }

    for (int arg = first_nonoption; arg < argc; arg++) {
        this->config.set_input_file( std::string(argv[arg]) );
        this->config.run_job();
    }
}

Garage::~Garage() {
    DEBUG("Destructing garage");
}

#include <libgen.h>

char *get_dir_name(char *in) throw() {
    return dirname(in);
}
