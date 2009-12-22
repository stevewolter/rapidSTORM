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

static void install_segmentation_fault_handler() {
    signal( SIGSEGV, sigsegv );
}

extern void foo();

int main(int argc, char *argv[]) {
    ost::DebugStream::set(cerr);
    DEBUG("entry: main");
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
