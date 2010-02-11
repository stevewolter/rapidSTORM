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

#include "error_handler.h"

#include "debug.h"

using namespace dStorm;
using namespace std;
using namespace cimg_library;

extern void foo();

void run_dstorm(int argc, char *argv[]) {
    DEBUG("entry: main");

#ifdef HAVE_LIBGRAPHICSMAGICK__
    Magick::InitializeMagick(argv[0]);
#endif
    cimg::exception_mode() = 0U;         /* Do not show CImg errors in windows. */

    DEBUG("Constructing garage with size " << sizeof(Garage));
    Garage garage( argc, argv );
    DEBUG("Finished garage");
}

int main(int argc, char *argv[]) {
    DEBUG("entry: main");
    int exit_code = EXIT_SUCCESS;
    ost::DebugStream::set(cerr);

    try {
        SignalHandler outer_handler;
        SIGNAL_HANDLER_PANIC_POINT(outer_handler);
        run_dstorm(argc, argv);
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        exit_code = EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << PACKAGE_NAME << ": "
                  << e.what() << std::endl;
        exit_code = EXIT_FAILURE;
    }
    DEBUG("exit: main");

    return exit_code;
}
