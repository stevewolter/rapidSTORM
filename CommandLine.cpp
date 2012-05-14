#include "debug.h"

#include <simparm/Message.hh>
#include "CommandLine.h"
#include <vector>
#include <string>
#include <dStorm/Config.h>
#include "JobStarter.h"
#include <fstream>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include "InputStream.h"
#include <dStorm/JobMaster.h>
#include "ModuleLoader.h"
#include <simparm/IO.hh>
#include <simparm/command_line.hh>
#include <dStorm/display/Manager.h>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/thread/condition.hpp>
#include "job/Config.h"

namespace dStorm {

using namespace output;

class TransmissionTreePrinter 
: public simparm::TriggerEntry,
  simparm::Listener
{
    const job::Config &config;
    void operator()( const simparm::Event& );
    void printNode( 
        const output::OutputSource& src,
        int indent );
  public:
    TransmissionTreePrinter(const job::Config&);
};

class TwiddlerLauncher
: public simparm::TriggerEntry,
  simparm::Listener
{
    job::Config &config;
    MainThread& main_thread;
    void operator()( const simparm::Event& );
  public:
    TwiddlerLauncher(job::Config&, MainThread& main_thread);
    ~TwiddlerLauncher();
};

void CommandLine::parse( int argc, char *argv[] ) {
    TransmissionTreePrinter printer(config);
    TwiddlerLauncher launcher(config, main_thread);
    simparm::IO argument_parser(NULL,NULL);

    for ( int i = 0; i < argc; i++ ) {
        DEBUG("Argument " << i << " is '" << argv[i] << "'");
    }

    config.registerNamedEntries( argument_parser );
    argument_parser.push_back( printer );
    argument_parser.push_back( launcher );
    argument_parser.push_back( starter );
    if ( display::Manager::getSingleton().getConfig() )
        argument_parser.push_back( *display::Manager::getSingleton().getConfig() );

    int shift = find_config_file(argc,argv);
    argc -= shift;
    argv += shift;

    DEBUG("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = readConfig(argument_parser, argc, argv);
    }

    DEBUG("Processing nonoption arguments from " <<first_nonoption << " to " <<  argc );
    for (int arg = first_nonoption; arg < argc; arg++) {
        std::cerr << "Warning: Command line argument " << argv[arg] << " was ignored." << std::endl;
    }
    DEBUG("Finished processing commandline arguments");
}

int CommandLine::find_config_file( int argc, char* argv[] ) {
    int shift = 0;
    DEBUG("Checking for relevant environment variables");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    DEBUG("Checking for command line config file");
    while ( argc > 2 && std::string(argv[1]) == "--config" ) {
        bool successfully_opened = load_config_file(std::string(argv[2]));
        have_file = have_file || successfully_opened;
        if ( !successfully_opened )
            DEBUG("Skipped unreadable config file '" << argv[2] << "'");
        argc -= 2;
        shift += 2;
        argv[2] = argv[0];
        argv = argv + 2;
    }
    DEBUG("Checking for home directory config file");
    if ( !have_file && home != NULL )
        have_file = load_config_file( std::string(home) + "/.dstorm");
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = load_config_file( 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt");
    return shift;
}

bool CommandLine::load_config_file(
    const std::string& name
) {
    DEBUG("Opening config file " << name);
    std::ifstream config_file( name.c_str() );
    if ( !config_file )
        return false;
    else {
        while ( config_file ) {
            DEBUG("Processing command from " << name);
            try {
                config.processCommand( config_file );
            } catch (const std::runtime_error& e) {
                std::cerr << "Unable to read initialization file: " + std::string(e.what())  << std::endl;
            }
        }
        return true;
    }
}

CommandLine::CommandLine( MainThread& main_thread )
: starter( &main_thread ), main_thread( main_thread )
{
    starter.setConfig(config);
    add_modules( config );
    config.all_modules_loaded();
}
CommandLine::~CommandLine() {
}

TransmissionTreePrinter::TransmissionTreePrinter
    ( const job::Config& c )
: simparm::TriggerEntry("ShowTransmissionTree", 
                        "Print tree view of outputs"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c)
{
    receive_changes_from( value );
}

void TransmissionTreePrinter::operator()( const simparm::Event& )
{
    printNode( config.outputSource, 0 );
}

void TransmissionTreePrinter::printNode( 
    const output::OutputSource& src, int indent 
) {
    const std::string& name = src.getNode().getName();
    if ( indent < 2 )
        std::cout << ((indent)?" ": name ) << "\n";
    else {
        std::cout << std::string(indent-2, ' ') << "|-" << name << "\n";
    }
    const FilterSource* fwd = dynamic_cast<const FilterSource*>(&src);
    if (fwd != NULL) {
        for (FilterSource::const_iterator
                            i = fwd->begin(); i != fwd->end(); i++)
            printNode(**i, indent+2);
    }
}

TwiddlerLauncher::TwiddlerLauncher
    ( job::Config& c, MainThread& main_thread )
: simparm::TriggerEntry("TwiddlerControl", 
                "Read stdin/out for simparm control commands"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c),
  main_thread(main_thread)
{
    receive_changes_from( value );
}

void TwiddlerLauncher::operator()( const simparm::Event& )
{
    main_thread.connect_stdio( config );
}

TwiddlerLauncher::~TwiddlerLauncher() {}

}
