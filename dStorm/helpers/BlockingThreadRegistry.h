#ifndef DSTORM_BLOCKING_THREAD_REGISTRY
#define DSTORM_BLOCKING_THREAD_REGISTRY

#include <dStorm/helpers/thread.h>
#include <list>

namespace dStorm {

class BlockingThreadRegistry {
  public:
    class Handle {
        friend class BlockingThreadRegistry;
        BlockingThreadRegistry& parent;
        std::list<Handle*>::iterator me;
        ost::Thread& thread;
      public:
        Handle(BlockingThreadRegistry&, ost::Thread&);
        ~Handle();
    };

    void cancel_blocking_threads();

  private:
    ost::Mutex mutex;
    std::list<Handle*> blocking_threads;
};

}

#endif
