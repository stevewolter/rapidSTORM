#include <semaphore.h>

namespace dStorm {
namespace helpers {

void set_semaphore_for_report_of_dead_threads( sem_t* s );

enum ThreadStage { BeforeDestruction, AfterDestruction };
void set_error_cleanup_for_threads( 
    void (*f)(ThreadStage, void *, Thread *),
    void *arg );

}
}
