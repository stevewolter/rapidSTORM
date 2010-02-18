#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CommandLine.h"
#include "ModuleLoader.h"
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

int main(int argc, char *argv[]) {
    DEBUG("entry: main");
    int exit_code = EXIT_SUCCESS;

    ost::DebugStream::set(cerr);
#ifdef HAVE_LIBGRAPHICSMAGICK__
    Magick::InitializeMagick(argv[0]);
#endif
    cimg::exception_mode() = 0U;         /* Do not show CImg errors in windows. */

    try {
        SignalHandler outer_handler;
        SIGNAL_HANDLER_PANIC_POINT(outer_handler);
        DEBUG("Running from panic point on");
        ModuleLoader::makeSingleton();

        (new CommandLine( argc, argv ))->detach();
        Thread::wait_for_detached_threads();

        ModuleLoader::destroySingleton();
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        exit_code = EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << PACKAGE_NAME << ": "
                  << e.what() << std::endl;
        exit_code = EXIT_FAILURE;
    } catch (...) {
        std::cerr << PACKAGE_NAME << ": Unhandled exception."
                  << std::endl;
        exit_code = EXIT_FAILURE;
    }
    DEBUG("exit: main");

    return exit_code;
}
