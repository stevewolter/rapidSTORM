#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <wx/app.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/imagpng.h>

#include "CommandLine.h"
#include <stdexcept>
#include <dStorm/helpers/thread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <locale.h>
#include "simparm/wx_ui/App.h"
#include "installation-directory.h"
#include <dStorm/GUIThread.h>
#include <fstream>
#include <simparm/GUILabelTable.h>
#include <boost/filesystem/fstream.hpp>
#include "config_file.h"

#ifdef USE_GRAPHICSMAGICK
#include <Magick++.h>
#endif

using namespace dStorm;
using namespace std;

static char english_env[] = "LC_ALL=C";

void start_imagemagick( const char* name ) {
#ifdef USE_GRAPHICSMAGICK
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
    ios_base::sync_with_stdio(false);
    ost::DebugStream::set(cerr);
    wxFileSystem::AddHandler(new wxArchiveFSHandler);
    wxImage::AddHandler(new wxPNGHandler);
    start_imagemagick( argv[0] );

    try {
        int success = parse_command_line( argc, argv );
        if ( success != EXIT_SUCCESS ) return success;

        GUIThread& main_thread = GUIThread::get_singleton();
        if ( main_thread.need_wx_widgets() ) {
            int wx_argc = 1;
            char* wx_argv[1] = { argv[0] };
            wxEntry( wx_argc, wx_argv );
        }
        main_thread.run_all_jobs();
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        return EXIT_FAILURE;
    } catch (const std::runtime_error& e) {
        std::cerr << PACKAGE_NAME << ": "
                  << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
