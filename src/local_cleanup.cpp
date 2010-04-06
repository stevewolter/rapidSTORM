#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "local_cleanup.h"
#include "AndorCamera/EmergencyHandler.h"
#include "InputStream.h"

void local_cleanup( dStorm::ErrorHandler::CleanupArgs& args, 
                    std::auto_ptr<dStorm::JobMaster>& master ) 
{
    if ( args.front() == "--Twiddler" ) {
        std::cout << "clear" << std::endl;
        std::auto_ptr<dStorm::InputStream> is(
            new dStorm::InputStream(NULL, &std::cout) );
        master.reset(is.release());
        args.pop_front();
    } else {
#ifdef HAVE_LIBATMCD32D
        AndorCamera::EmergencyHandler::do_emergency_cleanup( args, *master );
#endif
    }
}
