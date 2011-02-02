#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "CommandLine.h"
#include "ModuleLoader.h"
#include <stdexcept>
#include <dStorm/helpers/thread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <locale.h>
#include <stdlib.h>

#ifdef HAVE_MAGICK___H
#include <Magick++.h>
#endif

#include <dStorm/error_handler.h>

#include "debug.h"

#include "pthread.h"

using namespace dStorm;
using namespace std;

int main(int argc, char *argv[]) {
    DEBUG("entry: main");
    int exit_code = EXIT_SUCCESS;

    ost::DebugStream::set(cerr);
#ifdef HAVE_LIBGRAPHICSMAGICK__
#ifdef HAVE_MAGICK___H
    char english_env[] = { "LC_ALL=en_US" };
    /* Magick cannot save images in the de_DE locale. */
    putenv(english_env);
    Magick::InitializeMagick(argv[0]);
#endif
#endif

    const char *panic_mode = "--panic_mode";

    std::auto_ptr<MayBeASignal> rv;
    try {
        DEBUG("Constructing error handler");
        ErrorHandler outer_handler(argv[0], panic_mode);
        DEBUG("Making module handler");
        ModuleLoader::makeSingleton();

        if ( argc < 2 || string(argv[1]) != panic_mode ) {
            DEBUG("Running normally");
            std::auto_ptr<CommandLine> cmd_line;
            cmd_line.reset( new CommandLine( argc, argv ) );
            ModuleLoader::getSingleton().add_jobs( *cmd_line );
            cmd_line.release()->detach();
        } else {
            std::cerr << "Received unrecoverable signal " 
                      << argv[2] << ". Running emergency handler and "
                      << "terminating program. Sorry." << std::endl;
            rv.reset( new MayBeASignal( atoi(argv[2]) ) );
            exit_code = EXIT_FAILURE;
        }
        MayBeASignal my_signal =
            outer_handler.handle_errors_until_all_detached_threads_quit();
        if ( rv.get() == NULL ) 
            rv.reset( new MayBeASignal(my_signal) );

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

    if ( rv.get() && rv->did_receive_signal() )
#ifndef PTW32_VERSION
        kill( getpid(), rv->signal_number() );
#else
        return 2;
#endif
    return exit_code;
}
