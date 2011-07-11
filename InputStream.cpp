#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include <dStorm/doc/rapidstorm_help_file.h>
#include "JobStarter.h"
#include "engine/Car.h"

#include <simparm/IO.hh>

#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {

struct InputStream::Pimpl
: public simparm::IO,
  JobMaster
{
    InputStream& impl_for;
    ost::Mutex mutex;
    ost::Condition all_cars_finished;
    typedef std::set<Job*> Jobs;
    Jobs running_cars;

    bool exhausted_input;

    std::auto_ptr<Config> original;
    std::auto_ptr<Config> config;
    std::auto_ptr<JobStarter> starter;
    simparm::Attribute<std::string> help_file;
    boost::thread input_watcher;

    Pimpl(InputStream& papa, const Config*, 
          std::istream*, std::ostream*);
    ~Pimpl();

    void register_node( Job& );
    void erase_node( Job& );

    void run();

    void terminate_remaining_cars();

    void processCommand(const std::string& cmd, std::istream& rest);

    void reset_config();
};

InputStream::InputStream(
    const Config& c,
    std::istream& i, std::ostream& o)
: pimpl( new Pimpl(*this, &c, &i, &o) )
{
}

InputStream::InputStream(std::istream* i, std::ostream* o)
: pimpl( new Pimpl(*this, NULL, i, o) )
{
}

InputStream::~InputStream() 
{
}

InputStream::Pimpl::Pimpl(
    InputStream& impl_for,
    const Config* c,
    std::istream* i, std::ostream* o)
: simparm::IO(i,o),
  impl_for( impl_for ),
  all_cars_finished( mutex ),
  exhausted_input( i == NULL ),
  original( (c) ? new Config(*c) : NULL ),
  starter( (original.get()) ? new JobStarter(this) : NULL ),
  help_file("help_file", dStorm::HelpFileName)
{
    this->showTabbed = true;
    setDesc( ModuleLoader::getSingleton().makeProgramDescription() );
    this->push_back( help_file );
    reset_config();
    if ( Display::Manager::getSingleton().getConfig() )
        this->push_back( *Display::Manager::getSingleton().getConfig() );
}

void InputStream::Pimpl::reset_config() {
    ost::MutexLock lock(mutex);
    if ( original.get() ) {
        config.reset( new Config(*original) );
        this->push_back( *config );
        config->push_back( *starter );
        starter->setConfig( *config );
    }
}

void InputStream::Pimpl::terminate_remaining_cars() {
    ost::MutexLock lock(mutex);
    DEBUG("Terminate remaining cars");
    for ( Jobs::iterator i = running_cars.begin();
          i != running_cars.end(); ++i )
    {
        DEBUG("Sending stop to " << (*i)->get_config().getName());
        (*i)->stop();
        DEBUG("Sent stop to " << (*i)->get_config().getName());
    }
}

InputStream::Pimpl::~Pimpl() 
{
    DEBUG("Destroying InputStream::Pimpl");
    input_watcher.join();
    DEBUG("Joined InputStream::Pimpl subthread");
    terminate_remaining_cars();
    while ( ! running_cars.empty() )
        all_cars_finished.wait();
}

void InputStream::Pimpl::run() {
    DEBUG("Running input processing loop");
    std::cout << "# rapidSTORM waiting for commands" << std::endl;
    while ( !exhausted_input ) {
        try {
            DEBUG("Processing input");
            processInput();
            DEBUG("Processed input");
            exhausted_input = true;
        } catch (const std::bad_alloc& e) {
            std::cerr << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::runtime_error& e) {
            std::cerr << "Could not perform action: "
                      << e.what() << std::endl;
        }
    }
}

void InputStream::register_node( Job& node ) {pimpl->register_node(node);}
void InputStream::Pimpl::register_node( Job& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::push_back( node.get_config() );
    if ( node.needs_stopping() )
        running_cars.insert( &node );
}

void InputStream::erase_node( Job& node ) {pimpl->erase_node(node);}
void InputStream::Pimpl::erase_node( Job& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::erase( node.get_config() );
    running_cars.erase( &node );
    if ( running_cars.empty() )
        all_cars_finished.signal();
}

void InputStream::Pimpl::processCommand
    (const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "wait_for_jobs" ) {
        DEBUG("Got command to wait for jobs");
        terminate_remaining_cars();
        DEBUG("Terminated remaining cars");
        ost::MutexLock lock(mutex);
        while ( ! running_cars.empty() )
            all_cars_finished.wait();
        DEBUG("Processed command to wait for jobs");
    } else if ( cmd == "reset" ) {
        reset_config();
    } else
        simparm::IO::processCommand(cmd, rest);
}

void InputStream::start() {
    pimpl->input_watcher = boost::thread( &InputStream::Pimpl::run, pimpl.get() );
}

}
