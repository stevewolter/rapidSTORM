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
    std::list<Job*> &jobs;
    boost::ptr_vector<InputStream> streams;
    void operator()( const simparm::Event& );
  public:
    TwiddlerLauncher(job::Config&, std::list<Job*>&);
    ~TwiddlerLauncher();
};

class CommandLine::Pimpl
: public JobMaster
{
    simparm::IO io;
    int argc;
    char **argv;
    job::Config config;
    JobStarter starter;
    boost::mutex mutex;
    boost::condition jobs_empty;
    std::list<Job*> jobs;

    bool load_config_file(const std::string& filename);
    void find_config_file();

    class JobHandle : public dStorm::JobHandle {
        Pimpl& master;
        dStorm::Job& job;
        bool registered;
        ~JobHandle() { unregister_node(); master.deleted_node(job); }
        void unregister_node() { if ( registered ) { master.erase_node(job); registered = false; } }
      public:
        JobHandle( Pimpl& m, dStorm::Job& j ) : master(m), job(j), registered(true) {}
    };

  public:
    Pimpl(int argc, char *argv[]);
    ~Pimpl();

    void run();
    std::auto_ptr<dStorm::JobHandle> register_node( dStorm::Job& j ) { 
        DEBUG("Waiting for mutex to add job");
        boost::lock_guard<boost::mutex> lock(mutex);
        io.push_back(j.get_config());
        DEBUG("Pushed back " << j.get_config().getName());
        jobs.push_back( &j ); 
        return std::auto_ptr<dStorm::JobHandle>( new JobHandle( *this, j ) );
    }
    void erase_node( dStorm::Job& j ) {
        DEBUG("Waiting for mutex to delete job for " << j.get_config().getName());
        boost::lock_guard<boost::mutex> lock(mutex);
        io.erase(j.get_config());  
        DEBUG("Erased " << j.get_config().getName());
    }
    void deleted_node( dStorm::Job& j ) {
        boost::lock_guard<boost::mutex> lock(mutex);
        jobs.remove( &j ); 
        jobs_empty.notify_all();
    }
};

CommandLine::CommandLine(int argc, char *argv[])
: pimpl(new Pimpl(argc, argv))
{
}
CommandLine::~CommandLine()
{}

void CommandLine::run() {
    pimpl->run();
}

void CommandLine::Pimpl::run() {
    for ( int i = 0; i < argc; i++ ) {
        DEBUG("Argument " << i << " is '" << argv[i] << "'");
    }

    io.push_back(
        std::auto_ptr<simparm::Node>(new
            TransmissionTreePrinter(config)));
    io.push_back(
        std::auto_ptr<simparm::Node>(new
            TwiddlerLauncher(config, jobs)));
    io.push_back( starter );
    if ( display::Manager::getSingleton().getConfig() )
        io.push_back( *display::Manager::getSingleton().getConfig() );

    find_config_file();

    DEBUG("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = readConfig(io, argc, argv);
    }

    DEBUG("Processing nonoption arguments from " <<first_nonoption << " to " <<  argc );
    for (int arg = first_nonoption; arg < argc; arg++) {
        std::cerr << "Warning: Command line argument " << argv[arg] << " was ignored." << std::endl;
        //config.inputConfig.input_file() = std::string(argv[arg]);
        //starter.trigger();
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
    while ( argc > 2 && std::string(argv[1]) == "--config" ) {
        bool successfully_opened = load_config_file(std::string(argv[2]));
        have_file = have_file || successfully_opened;
        if ( !successfully_opened )
            DEBUG("Skipped unreadable config file '" << argv[2] << "'");
        argc -= 2;
        argv[2] = argv[0];
        argv = argv + 2;
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

CommandLine::Pimpl::Pimpl(int argc, char *argv[])
: io(NULL, NULL), argc(argc), argv(argv), starter(this)
{
    starter.setConfig(config);
    ModuleLoader::getSingleton().add_modules( config );
    config.all_modules_loaded();
    config.registerNamedEntries(io);
}
CommandLine::Pimpl::~Pimpl() {
    boost::unique_lock<boost::mutex> lock(mutex);
    while ( ! jobs.empty() ) {
        if ( ! jobs.empty() ) {
            DEBUG("Waiting with " << jobs.size() << " jobs");
            jobs_empty.wait(lock);
        }
    }
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
    ( job::Config& c, std::list<Job*>& j )
: simparm::TriggerEntry("TwiddlerControl", 
                "Read stdin/out for simparm control commands"),
  simparm::Listener( simparm::Event::ValueChanged ),
  config(c),
  jobs(j)
{
    receive_changes_from( value );
}

void TwiddlerLauncher::operator()( const simparm::Event& )
{
    DEBUG("Launching command stream");
    std::auto_ptr<InputStream> is(new InputStream(config, std::cin, std::cout));
    for ( std::list<Job*>::iterator i = jobs.begin(); i != jobs.end(); i++ ) {
        DEBUG("Registering additional job " << (*i)->get_config().getName());
        is->register_node( **i );
    }
    is->start();
    streams.push_back( is );
    DEBUG("Launched command stream");
}

TwiddlerLauncher::~TwiddlerLauncher() {}

std::auto_ptr<JobHandle> CommandLine::register_node( Job& j ) { return pimpl->register_node( j ); }

}
