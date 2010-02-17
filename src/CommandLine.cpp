#include "CommandLine.h"
#include <vector>
#include <string>
#include "engine/CarConfig.h"
#include "JobStarter.h"
#include <fstream>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/FilterSource.h>
#include "InputStream.h"
#include "JobMaster.h"
#include "ModuleLoader.h"

#include "debug.h"

namespace dStorm {

using namespace output;

class TransmissionTreePrinter 
: public simparm::TriggerEntry,
  simparm::Listener,
  boost::noncopyable
{
    const engine::CarConfig &config;
    void operator()( const simparm::Event& );
    void printNode( 
        const output::OutputSource& src,
        int indent );
  public:
    TransmissionTreePrinter(const engine::CarConfig&);
};

class TwiddlerLauncher
: public simparm::TriggerEntry,
  simparm::Listener,
  boost::noncopyable
{
    engine::CarConfig &config;
    void operator()( const simparm::Event& );
  public:
    TwiddlerLauncher(engine::CarConfig&);
};

class CommandLine::Pimpl
: public JobMaster {
    int argc;
    char **argv;
    engine::CarConfig config;
    JobStarter starter;

    bool load_config_file(const std::string& filename);
    void find_config_file();

  public:
    Pimpl(int argc, char *argv[]);

    void run();
    void register_node( engine::Car& ) {}
    void erase_node( engine::Car&  )  {}
};

CommandLine::CommandLine(int argc, char *argv[])
: Thread("CommandLineInterpreter"),
  pimpl(new Pimpl(argc, argv))
{
}
CommandLine::~CommandLine()
{}

void CommandLine::run() {
    pimpl->run();
}

void CommandLine::abnormal_termination(std::string reason) {
    std::cerr << "Had an unexpected error while processing command line "
                 "arguments: " << reason << " Aborting." << std::endl;
    this->exit();
}
void CommandLine::Pimpl::run() {
    simparm::Set cmd_line_args
        ("dSTORM", "dSTORM command line");
    cmd_line_args.push_back( config );
    cmd_line_args.push_back(
        std::auto_ptr<simparm::Node>(new
            TransmissionTreePrinter(config)));
    cmd_line_args.push_back(
        std::auto_ptr<simparm::Node>(new
            TwiddlerLauncher(config)));
    cmd_line_args.push_back( starter );

    find_config_file();

    DEBUG("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = 
            cmd_line_args.readConfig(argc, argv);
    }

    DEBUG("Processing nonoption arguments from " <<first_nonoption << " to " <<  argc );
    for (int arg = first_nonoption; arg < argc; arg++) {
        config.inputConfig.inputFile = std::string(argv[arg]);
        starter.trigger();
    }
    DEBUG("Finished processing commandline arguments");
}

void CommandLine::Pimpl::find_config_file() {
    DEBUG("Checking for relevant environment variables");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    DEBUG("Checking for command line config file");
    if ( !have_file && argc > 2 && std::string(argv[1]) == "--config" ) {
        have_file = load_config_file(std::string(argv[2]));
        if ( have_file ) {
            std::cerr << "Have a file here" <<std::endl;
            argc -= 2;
            argv = argv + 2;
        }
    }
    DEBUG("Checking for home directory config file");
    if ( !have_file && home != NULL )
        have_file = load_config_file( std::string(home) + "/.dstorm");
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = load_config_file( 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt");

}

bool CommandLine::Pimpl::load_config_file(
    const std::string& name
) {
    std::ifstream config_file( name.c_str() );
    if ( !config_file )
        return false;
    else {
        while ( config_file ) {
            DEBUG("Processing command from " << name.c_str());
            try {
                config.processCommand( config_file );
            } catch (const std::exception& e) {
                std::cerr << "Error in initialization config file: "
                          << e.what() << "\n";
            }
        }
        return true;
    }
}

CommandLine::Pimpl::Pimpl(int argc, char *argv[])
: argc(argc), argv(argv), starter(config, *this)
{
    ModuleLoader::getSingleton().add_modules( config );
}

TransmissionTreePrinter::TransmissionTreePrinter
    ( const engine::CarConfig& c )
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
    ( engine::CarConfig& c )
: simparm::TriggerEntry("TwiddlerControl", 
                "Read stdin/out for simparm control commands"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c)
{
    receive_changes_from( value );
}

void TwiddlerLauncher::operator()( const simparm::Event& )
{
    DEBUG("Launching command stream");
    std::auto_ptr<InputStream> is(new InputStream(config, std::cin, std::cout));
    is.release()->detach();
    DEBUG("Launched command stream");
}

}
