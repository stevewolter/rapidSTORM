#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "JobStarter.h"
#include "job/Config.h"
#include "test-plugin/cpu_time.h"

#include <simparm/IO.hh>

#include <dStorm/display/Manager.h>
#include <dStorm/stack_realign.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <dStorm/helpers/thread.h>

namespace dStorm {

struct InputStream::Pimpl
: public simparm::IO,
  JobMaster
{
    InputStream& impl_for;
    boost::recursive_mutex mutex;
    boost::condition all_cars_finished;
    typedef std::set<Job*> Jobs;
    Jobs running_cars, stopping_cars;

    bool exhausted_input;

    std::auto_ptr<job::Config> original;
    std::auto_ptr<job::Config> config;
    std::auto_ptr<JobStarter> starter;
    boost::thread input_watcher;

    class JobHandle : public dStorm::JobHandle {
        Pimpl& master;
        dStorm::Job& job;
        bool registered;
        ~JobHandle() { unregister_node(); master.deleted_node(job); }
        void unregister_node() { if ( registered ) { master.erase_node(job); registered = false; } }
      public:
        JobHandle( Pimpl& m, dStorm::Job& j ) : master(m), job(j), registered(true) {}
    };

    Pimpl(InputStream& papa, const job::Config*, 
          std::istream*, std::ostream*);
    ~Pimpl();

    std::auto_ptr<dStorm::JobHandle> register_node( Job& );
    void erase_node( Job& );
    void deleted_node( Job& );

    DSTORM_REALIGN_STACK void run();

    void terminate_remaining_cars();

    void processCommand(const std::string& cmd, std::istream& rest);

    void reset_config();
    void print(const std::string& what);
};

void InputStream::Pimpl::print(const std::string& what) {
    ost::DebugStream::get()->ost::LockedStream::begin();
    simparm::IO::print(what);
    ost::DebugStream::get()->ost::LockedStream::end();
}

InputStream::InputStream(
    const job::Config& c,
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
    const job::Config* c,
    std::istream* i, std::ostream* o)
: simparm::IO(i,o),
  impl_for( impl_for ),
  exhausted_input( i == NULL ),
  original( (c) ? new job::Config(*c) : NULL ),
  starter( (original.get()) ? new JobStarter(this) : NULL )
{
    this->showTabbed = true;
    setDesc( ModuleLoader::getSingleton().makeProgramDescription() );
    reset_config();
    if ( display::Manager::getSingleton().getConfig() )
        this->push_back( *display::Manager::getSingleton().getConfig() );
}

void InputStream::Pimpl::reset_config() {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if ( original.get() ) {
        config.reset( new job::Config(*original) );
        config->registerNamedEntries( *this );
        config->push_back( *starter );
        starter->setConfig( *config );
    }
}

void InputStream::Pimpl::terminate_remaining_cars() {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
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
    boost::unique_lock<boost::recursive_mutex> lock(mutex);
    while ( ! running_cars.empty() || ! stopping_cars.empty() )
        all_cars_finished.wait(mutex);
}

void InputStream::Pimpl::run()
{
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

std::auto_ptr<dStorm::JobHandle> InputStream::register_node( Job& node ) { return pimpl->register_node(node);}
std::auto_ptr<dStorm::JobHandle> InputStream::Pimpl::register_node( Job& node ) {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    simparm::Node::push_back( node.get_config() );
    if ( node.needs_stopping() )
        running_cars.insert( &node );
    return std::auto_ptr<dStorm::JobHandle>( new JobHandle(*this, node) );
}

void InputStream::Pimpl::erase_node( Job& node ) {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    DEBUG("Erasing node " << node.get_config().getName());
    running_cars.erase( &node );
    stopping_cars.insert( &node );
}

void InputStream::Pimpl::deleted_node( Job& node ) {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    DEBUG("Deleting node " << node.get_config().getName() << " from list");
    stopping_cars.erase( &node );
    if ( running_cars.empty() && stopping_cars.empty() )
        all_cars_finished.notify_all();
}

void InputStream::Pimpl::processCommand
    (const std::string& cmd, std::istream& rest)
{
    DEBUG("Processing command " << cmd);
    if ( cmd == "wait_for_jobs" ) {
        DEBUG("Got command to wait for jobs");
        terminate_remaining_cars();
        DEBUG("Terminated remaining cars");
        boost::unique_lock<boost::recursive_mutex> lock(mutex);
        while ( ! running_cars.empty() )
            all_cars_finished.wait( lock );
        DEBUG("Processed command to wait for jobs");
    } else if ( cmd == "resource_usage" ) {
        boost::optional<double> cpu_time = get_cpu_time();
        if ( cpu_time )
            std::cout << "Current CPU time: " << *cpu_time << std::endl;
        else
            std::cout << "Resource usage not supported" << std::endl;
    } else if ( cmd == "reset" ) {
        reset_config();
    } else if ( cmd == "cmd" ) {
        simparm::IO::processCommand(cmd, rest);
    } else {
        boost::unique_lock<boost::recursive_mutex> lock(mutex);
        simparm::IO::processCommand(cmd, rest);
    }
}

void InputStream::start() {
    pimpl->input_watcher = boost::thread( &InputStream::Pimpl::run, pimpl.get() );
}

}
