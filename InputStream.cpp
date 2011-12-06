#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"
#include "InputStream.h"
#include "ModuleLoader.h"
#include "JobStarter.h"
#include "Car.h"

#include <simparm/IO.hh>

#include <dStorm/helpers/DisplayManager.h>

#if HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#include <string.h>
#if HAVE_ERRNO_H
#include <errno.h>
#endif

namespace dStorm {

struct InputStream::Pimpl
: public simparm::IO,
  JobMaster
{
    InputStream& impl_for;
    ost::Mutex mutex;
    ost::Condition all_cars_finished;
    typedef std::set<Job*> Jobs;
    Jobs running_cars, stopping_cars;

    bool exhausted_input;

    std::auto_ptr<Config> original;
    std::auto_ptr<Config> config;
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

    Pimpl(InputStream& papa, const Config*, 
          std::istream*, std::ostream*);
    ~Pimpl();

    std::auto_ptr<dStorm::JobHandle> register_node( Job& );
    void erase_node( Job& );
    void deleted_node( Job& );

    DSTORM_REALIGN_STACK void run();

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
  starter( (original.get()) ? new JobStarter(this) : NULL )
{
    this->showTabbed = true;
    setDesc( ModuleLoader::getSingleton().makeProgramDescription() );
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
    while ( ! running_cars.empty() || ! stopping_cars.empty() )
        all_cars_finished.wait();
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
    ost::MutexLock lock(mutex);
    simparm::Node::push_back( node.get_config() );
    if ( node.needs_stopping() )
        running_cars.insert( &node );
    return std::auto_ptr<dStorm::JobHandle>( new JobHandle(*this, node) );
}

void InputStream::Pimpl::erase_node( Job& node ) {
    ost::MutexLock lock(mutex);
    DEBUG("Erasing node " << node.get_config().getName());
    simparm::Node::erase( node.get_config() );
    running_cars.erase( &node );
    stopping_cars.insert( &node );
}

void InputStream::Pimpl::deleted_node( Job& node ) {
    ost::MutexLock lock(mutex);
    DEBUG("Deleting node " << node.get_config().getName() << " from list");
    stopping_cars.erase( &node );
    if ( running_cars.empty() && stopping_cars.empty() )
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
    } else if ( cmd == "resource_usage" ) {
#if HAVE_GETRUSAGE
        struct rusage usage;
        int rv = getrusage(RUSAGE_SELF, &usage);
        if ( rv == -1 ) {
            std::cout << "Getting resource usage failed: " 
                      << strerror(errno) << std::endl;
        } else {
            std::cout << "Current CPU time: " << usage.ru_utime.tv_sec << "."
                                              << usage.ru_utime.tv_usec << "\n";
        }
#else
        std::cout << "Resource usage not supported" << std::endl;
#endif
    } else if ( cmd == "reset" ) {
        reset_config();
    } else
        simparm::IO::processCommand(cmd, rest);
}

void InputStream::start() {
    pimpl->input_watcher = boost::thread( &InputStream::Pimpl::run, pimpl.get() );
}

}
