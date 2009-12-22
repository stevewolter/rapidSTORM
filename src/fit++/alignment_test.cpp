#include <dStorm/helpers/thread.h>
#include <stdio.h>
#include <Eigen/Core>
#include <cassert>

struct T : public ost::Thread {
  T() : ost::Thread("Test") {}
  void run() throw() {
    EIGEN_ALIGN_128 int bar;
    assert( /* Integer is properly aligned */ 
        ((int)(&bar) & (0xf)) == 0 );
  }
};

int main() {
    T t;
    t.start();
    t.join();
    return 0;
}
