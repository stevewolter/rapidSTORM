#include <boost/thread/thread.hpp>
#include <stdio.h>
#include <Eigen/Core>
#include <cassert>

void run() {
    EIGEN_ALIGN_128 int bar;
    assert( /* Integer is properly aligned */ 
        ((int)(&bar) & (0xf)) == 0 );
}

int main() {
    boost::thread thread;
    thread = boost::thread( &run );
    thread.join();
    return 0;
}
