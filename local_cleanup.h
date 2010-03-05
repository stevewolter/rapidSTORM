#ifndef LOCAL_CLEANUP_H
#define LOCAL_CLEANUP_H

#include <dStorm/error_handler.h>
#include <dStorm/JobMaster.h>

void local_cleanup( 
    dStorm::ErrorHandler::CleanupArgs& args, 
    std::auto_ptr<dStorm::JobMaster>& master );

#endif
