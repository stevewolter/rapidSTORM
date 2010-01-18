#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Garage.h"
#include <stdexcept>
#include <signal.h>
#include <dStorm/helpers/thread.h>
#include <stdlib.h>

#ifdef HAVE_LIBGRAPHICSMAGICK__
#include <Magick++.h>
#endif
#include <CImg.h>

#include "debug.h"

using namespace dStorm;
using namespace std;
using namespace cimg_library;

static void sigsegv(int) {
    DEBUG("Caught sigsegv. Throwing exception and hoping for the best.");
    throw std::runtime_error("Segmentation fault. This is a severe error, "
                             "with no information on it's source. Sorry "
                             "for the inconvenience.");
}

static void sigfpe(int) {
    DEBUG("Caught sigfpe. Throwing exception and hoping for the best.");
    throw std::runtime_error("Floating-point exception. "
                             "This is a severe error, "
                             "with no information on it's source. Sorry "
                             "for the inconvenience.");
}
static void sigabort(int) {
    std::cerr << "Sigabort\n";
    signal( SIGSEGV, SIG_DFL );
    signal( SIGABRT, SIG_DFL );
    *(int*)(0x23) = 5;
}

static void install_segmentation_fault_handler() {
    signal( SIGSEGV, sigsegv );
    signal( SIGFPE, sigfpe );
    signal( SIGABRT, sigabort );
}

extern void foo();

int main(int argc, char *argv[]) {
    ost::DebugStream::set(cerr);
    DEBUG("entry: main");
    if ( argc <= 1 )
        install_segmentation_fault_handler();
    else if ( !strcmp( argv[1], "---no-catch" ) ) {
        std::cerr << "Ignoring signal handling\n";
        argv[1] = argv[0];
        argc--;
        argv++;
    } else
        install_segmentation_fault_handler();

#ifdef HAVE_LIBGRAPHICSMAGICK__
    Magick::InitializeMagick(argv[0]);
#endif
    cimg::exception_mode() = 0U;         /* Do not show CImg errors in windows. */

    int exit_code = EXIT_SUCCESS;
    try {
        DEBUG("Constructing garage with size " << sizeof(Garage));
        Garage garage( argc, argv );
        DEBUG("Finished garage");
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        exit_code = EXIT_FAILURE;
    } catch (const std::exception &e) {
        cerr << PACKAGE_NAME << ": " << e.what() << endl;
        exit_code = EXIT_FAILURE;
    }

    DEBUG("Waiting for thread termination");
    ost::Thread::joinDetached();
    DEBUG("Joined all threads.");
    DEBUG("exit: main");
    return EXIT_SUCCESS;
}
