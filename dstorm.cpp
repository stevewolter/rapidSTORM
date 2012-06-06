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
#include <wx/app.h>
#include "wxDisplay/App.h"

#include <dStorm/display/Manager.h>
#include "test-plugin/plugin.h"
#include "wxDisplay/fwd.h"
#include "wxDisplay/App.h"

#ifdef USE_GRAPHICSMAGICK
#include <Magick++.h>
#endif

#include "debug.h"

#include "unit_tests.h"

using namespace dStorm;
using namespace std;

void start_imagemagick( const char* name ) {
#ifdef USE_GRAPHICSMAGICK
    char english_env[] = { "LC_ALL=C" };
    /* Magick cannot save images in the de_DE locale. */
    putenv(english_env);
    Magick::InitializeMagick(name);
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

}

int main(int argc, char *argv[]) {
    DEBUG("entry: main");

    ios_base::sync_with_stdio(false);
    ost::DebugStream::set(cerr);
    start_imagemagick( argv[0] );

    try {
#if 0
        MainThread main_thread;
        CommandLine cmd_line(main_thread);
        cmd_line.parse( argc, argv );
        main_thread.run_all_jobs();
#endif
        int success = wxEntryStart( argc, argv );
        display::App app;
        app.run( success );
        if ( success ) wxEntryCleanup();
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        std::cerr << PACKAGE_NAME << ": "
                  << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
