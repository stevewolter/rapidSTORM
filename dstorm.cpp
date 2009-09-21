#include "Garage.h"
#include <stdexcept>
#include <signal.h>
#include <cc++/thread.h>
#include <stdlib.h>
#ifdef HAVE_LIBMAGICK__
#include <Magick++.h>
#endif
#include <CImg.h>

using namespace dStorm;
using namespace std;
using namespace cimg_library;

#if DEBUG_LEVEL >= 1
void sigsegv(int) {
    STATUS("Caught sigsegv");
    exit(1);
}
#endif

int main(int argc, char *argv[]) {
#ifdef HAVE_LIBMAGICK__
    Magick::InitializeMagick(argv[0]);
#endif
    cimg::exception_mode() = 0U;         /* Do not show CImg errors in windows. */

#if DEBUG_LEVEL >= 1
    STATUS("Debug version at level " << DEBUG_LEVEL);
    ost::DebugStream::set(cerr);
    //signal(SIGSEGV, sigsegv);
#endif
    try {
        STATUS("Constructing garage with size " << sizeof(Garage));
        Garage garage(argc, argv);
        STATUS("Finished garage");
    } catch (const std::bad_alloc &e) {
        std::cerr << PACKAGE_NAME << ": Ran out of memory" 
                  << std::endl;
        return EXIT_FAILURE;
    } catch (const std::exception &e) {
        cerr << PACKAGE_NAME << ": " << e.what() << endl;
        return EXIT_FAILURE;
    }

    STATUS("Waiting for thread termination");
    ost::Thread::joinDetached();
    STATUS("Joined all threads. Successful termination.");
    return EXIT_SUCCESS;
}
