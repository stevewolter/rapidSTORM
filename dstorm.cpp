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
    /* GraphicsMagick tends to fuck up signal handling, restore the signals. */
    int fuckedup[] = {
#ifdef SIGHUP
        SIGHUP,
#endif
#ifdef SIGINT
        SIGINT,
#endif
#ifdef SIGQUIT
        SIGQUIT,
#endif
#ifdef SIGABRT
        SIGABRT,
#endif 
#ifdef SIGFPE
        SIGFPE,
#endif
#ifdef SIGTERM
        SIGTERM
#endif
    };
    for (unsigned int i= 0; i < sizeof(fuckedup) / sizeof(int); ++i )
    signal(fuckedup[i], SIG_DFL);
#endif
#endif

    try {
        DEBUG("Making module handler");
        ModuleLoader::makeSingleton();

        std::auto_ptr<CommandLine> cmd_line;
        cmd_line.reset( new CommandLine( argc, argv ) );
        cmd_line->run();

        cmd_line.reset();
        ModuleLoader::destroySingleton();
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        exit_code = EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        std::cerr << PACKAGE_NAME << ": "
                  << e.what() << std::endl;
        exit_code = EXIT_FAILURE;
    }
    DEBUG("exit: main");

    return exit_code;
}
