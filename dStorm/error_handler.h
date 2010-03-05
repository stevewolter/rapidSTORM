#ifndef DSTORM_ERROR_HANDLER_H
#define DSTORM_ERROR_HANDLER_H

#include <memory>
#include <list>
#include <string>
#include <stdexcept>
#include <boost/shared_ptr.hpp>
#include "helpers/MayBeASignal.h"

namespace dStorm {

class BlockingThreadRegistry;

class ErrorHandler {
    struct Pimpl;
    std::auto_ptr<Pimpl> pimpl;

  public:
    static bool global_termination_flag;

    ErrorHandler(const char *program_path, const char *panic_mode_call);
    ~ErrorHandler();

    static ErrorHandler& get_current_handler();
    
    struct Cleanup;
    typedef boost::shared_ptr<Cleanup> CleanupTag;
    typedef std::list<std::string> CleanupArgs;
    struct Cleanup {
        struct Pimpl;
        std::auto_ptr<Pimpl> pimpl;
        Cleanup(ErrorHandler::Pimpl&, const CleanupArgs&);
        ~Cleanup();
    };
    static CleanupTag make_tag(const CleanupArgs& arguments)
        { return CleanupTag( new Cleanup(
            *get_current_handler().pimpl, arguments) ); }

    /** @return A pair indicating whether (first) a signal was received and
     *          which was sent (second). */
    MayBeASignal handle_errors_until_all_detached_threads_quit();
    std::auto_ptr<BlockingThreadRegistry> blocking_thread_registry;
};

}

#endif
