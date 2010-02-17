#include "InputStream.h"
#include "ModuleLoader.h"
#include "doc/help/rapidstorm_help_file.h"
#include "JobStarter.h"
#include "engine/Car.h"

#include <simparm/IO.hh>

namespace dStorm {

struct InputStream::Pimpl
: public dStorm::Thread,
  public simparm::IO,
  JobMaster
{
    InputStream& impl_for;
    ost::Mutex mutex;
    ost::Condition all_cars_finished;
    typedef std::set<engine::Car*> Jobs;
    Jobs running_cars;

    bool exhausted_input;

    engine::CarConfig config;
    JobStarter starter;
    simparm::Attribute<std::string> help_file;

    Pimpl(InputStream& papa, const engine::CarConfig&, 
          std::istream&, std::ostream&);
    ~Pimpl();

    void register_node( engine::Car& );
    void erase_node( engine::Car& );

    void run();
    void abnormal_termination(std::string abnormal_term);

    void terminate_remaining_cars();
};

InputStream::InputStream(
    const engine::CarConfig& c,
    std::istream& i, std::ostream& o)
: dStorm::Thread("InputWatcher"),
  pimpl( new Pimpl(*this, c, i, o) )
{
}

InputStream::~InputStream() {}

InputStream::Pimpl::Pimpl(
    InputStream& impl_for,
    const engine::CarConfig& c,
    std::istream& i, std::ostream& o)
: dStorm::Thread("InputProcessor"),
  simparm::IO(&i,&o),
  impl_for( impl_for ),
  all_cars_finished( mutex ),
  exhausted_input( false ),
  config(c),
  starter( config, *this ),
  help_file("help_file", dStorm::HelpFileName)
{
    this->showTabbed = true;
    setDesc( ModuleLoader::getSingleton().makeProgramDescription() );
    this->push_back( help_file );
    this->push_back( config );
    config.push_back( starter );
}

void InputStream::Pimpl::terminate_remaining_cars() {
    ost::MutexLock lock(mutex);
    for ( Jobs::iterator i = running_cars.begin();
          i != running_cars.end(); i++)
        (*i)->stop();
}

InputStream::Pimpl::~Pimpl() 
{
    terminate_remaining_cars();
    while ( ! running_cars.empty() )
        all_cars_finished.wait();
}

void InputStream::run() {
    while ( !pimpl->exhausted_input ) {
        pimpl->start();
        pimpl->join();
    }

    ost::MutexLock lock( pimpl->mutex );
}

void InputStream::abnormal_termination(std::string reason) {
    std::cerr << "The command stream managment thread had an unexpected "
                 "error: " << reason << " Command processing must be "
                 "abandoned; your interface will stop to react. Sorry."
              << std::endl;
    this->exit();
}

void InputStream::Pimpl::abnormal_termination(std::string reason) {
    std::cerr << "Could not perform action due to serious unhandled error: "
                 << reason << " I will recover this error and continue "
                 "running the program, but something is seriously broken "
                 "and it is quite likely that the program will behave "
                 "incorrectly now. Please report this bug." << std::endl;
    this->exit();
}

void InputStream::Pimpl::run() {
    while ( !exhausted_input ) {
        try {
            processInput();
            exhausted_input = true;
        } catch (const std::bad_alloc& e) {
            std::cerr << "Could not perform action: "
                      << "Out of memory" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Could not perform action: "
                      << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Could not perform action: "
                      << "Caught unknown exception."
                      << std::endl;
        }
    }
}

void InputStream::Pimpl::register_node( engine::Car& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::push_back( node.statusNode() );
    running_cars.insert( &node );
}

void InputStream::Pimpl::erase_node( engine::Car& node ) {
    ost::MutexLock lock(mutex);
    simparm::Node::erase( node.statusNode() );
    running_cars.erase( &node );
    if ( running_cars.empty() )
        all_cars_finished.signal();
}


}
