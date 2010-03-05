#include "../debug.h"
#include "BlockingThreadRegistry.h"

namespace dStorm {

BlockingThreadRegistry::Handle::Handle
    (BlockingThreadRegistry& p, ost::Thread& t)
: parent(p),
  thread(t)
{
  ost::MutexLock lock( p.mutex );
  DEBUG( "Making handle " << this << " for " << &t );
  me = p.blocking_threads.insert(p.blocking_threads.end(), this); 
}

BlockingThreadRegistry::Handle::~Handle() {
  ost::MutexLock lock( parent.mutex );
  DEBUG( "Destroying handle " << this << " for " << &thread );
  if ( me != parent.blocking_threads.end() )
    parent.blocking_threads.erase( me );
  DEBUG( "Destroyed handle " << this << " for " << &thread );
}

void BlockingThreadRegistry::cancel_blocking_threads() {
    ost::MutexLock lock( mutex );
    while ( ! blocking_threads.empty() ) {
        Handle* h = blocking_threads.front();
        DEBUG( "Cancelling handle " << h << " for " << &h->thread );
        h->thread.cancel();
        h->me = blocking_threads.end();
        blocking_threads.pop_front();
    }
}

}
