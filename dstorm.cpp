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

#include <dStorm/display/Manager.h>
#include "test-plugin/plugin.h"
#include "wxDisplay/fwd.h"

#ifdef USE_GRAPHICSMAGICK
#include <Magick++.h>
#endif

#include "debug.h"

#include "unit_tests.h"

using namespace dStorm;
using namespace std;

int main(int argc, char *argv[]) {
    DEBUG("entry: main");
    int exit_code = EXIT_SUCCESS;

    ios_base::sync_with_stdio(false);

    ost::DebugStream::set(cerr);
#ifdef USE_GRAPHICSMAGICK
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

    if ( argc <= 1 )
        return run_unit_tests(argc,argv);
    else if ( std::string(argv[1]) == "--version" ) {
        std::cout << PACKAGE_VERSION << std::endl;
        return 0;
    }

    try {
        std::auto_ptr< display::Manager > display( display::make_wx_manager() );
        display.reset( test::make_display( display.release() ) );
        display::Manager::setSingleton(*display);

        std::auto_ptr<CommandLine> cmd_line;
        cmd_line.reset( new CommandLine( argc, argv ) );
        cmd_line->run();
        cmd_line.reset();

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
