#include <boost/thread/mutex.hpp>
#include <boost/thread/locks.hpp>
#include <string>
#include <list>
#include <tiffio.h>
#include <stdarg.h>
#include "simparm/Message.h"
#include "simparm/NodeHandle.h"

namespace dStorm {

class TIFFOperation {
    static boost::mutex mutex;
    static TIFFOperation* current;

    boost::lock_guard<boost::mutex> lock;
    simparm::NodeHandle message_handler;
    std::list<simparm::Message> errors;
    const std::string error_title;

    TIFFErrorHandler old_warning_handler;
    TIFFErrorHandler old_error_handler;

    static void make_warning(const char *module, const char *fmt, va_list ap);
    static void make_error(const char *module, const char *fmt, va_list ap);
    static void ignore(const char *module, const char *fmt, va_list ap);

  public:
    TIFFOperation( 
        std::string error_title,
        simparm::NodeHandle message_handler,
        bool suppress_warnings );
    void throw_exception_for_errors();
    ~TIFFOperation();
};
}
