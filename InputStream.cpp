#include "InputStream.h"

#include <simparm/IO.hh>

namespace dStorm {

struct InputStream::Pimpl
: public dStorm::Thread,
  public simparm::IO
{
    ost::Mutex mutex;
    ost::Condition all_nodes_unregistered;
    int registered_nodes;

    bool exhausted_input;

    Pimpl(std::istream&, std::ostream&);

    void run();
    void abnormal_termination(std::string abnormal_term);
};

InputStream::InputStream(std::istream& i, std::ostream& o)
: dStorm::Thread("InputWatcher"),
  pimpl( new Pimpl(i,o) )
{
}

InputStream::Pimpl::Pimpl(std::istream& i, std::ostream& o)
: dStorm::Thread("InputProcessor"),
  simparm::IO(i,o),
  all_nodes_unregistered( mutex ),
  registered_nodes( 0 ),
  exhausted_input( false )
{
    this->showTabbed = true;
    setDesc( ModuleHandler::makeProgramDescription() );
}

void InputStream::run() {
    while ( !pimpl->exhausted_input ) {
        pimpl->start();
        pimpl->join();
    }

    ost::MutexLock lock( pimpl->mutex );
    while ( pimpl->registered_nodes > 0 )
        pimpl->all_nodes_unregistered.wait();
}

void InputStream::Pimpl::run() {
    while ( !exhausted_input ) {
        try {
            pimpl->processInput();
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

void InputStream::Pimpl::abnormal_termination(std::string r)
{
    std::cerr << "Could not perform action: " << r 
              << std::endl;
    this->exit();
}

void InputStream::thread_safely_register_node( simparm::Node& node ) {
    ost::MutexLock lock(mutex);
    this->simparm::Node::push_back( node );
    registered_nodes++;
}

void InputStream::thread_safely_erase_node( simparm::Node& node ) {
    ost::MutexLock lock(pimpl->mutex);
    this->simparm::Node::erase( node );
    --registered_nodes;
    if ( registered_nodes == 0 )
        all_modules_unregistered.signal();
}


}
