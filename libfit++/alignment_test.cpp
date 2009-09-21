#include <cc++/thread.h>
#include <stdio.h>
#include <Eigen/Core>
#include <assert>

class T : public Thread {
  void run() throw() {
    EIGEN_ALIGN_128 int bar;
    assert( /* Integer is properly aligned */ ((&bar) & 0xf) == 0 );
  }
}

int main() {
    T t;
    t.start();
    t.join();
    return 0;
}
