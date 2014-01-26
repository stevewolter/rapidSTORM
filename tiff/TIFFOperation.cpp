#include "TIFFOperation.h"
#include <simparm/Message.h>
#include <cassert>
#include <stdexcept>

namespace dStorm {

boost::mutex TIFFOperation::mutex;
TIFFOperation* TIFFOperation::current = NULL;
static bool suppress_TIFF_warnings = false;

TIFFOperation::TIFFOperation( 
    std::string error_title,
    simparm::NodeHandle message_handler, 
    bool suppress_warnings
) : lock(mutex),
    message_handler(message_handler) ,
    error_title(error_title)
{
    current = this;
    old_warning_handler = TIFFSetWarningHandler( make_warning );
    old_error_handler = TIFFSetErrorHandler( make_error );
    suppress_TIFF_warnings = suppress_warnings;
}

TIFFOperation::~TIFFOperation() {
    TIFFSetWarningHandler( old_warning_handler );
    TIFFSetErrorHandler( old_error_handler );
    current = NULL;
    while ( ! errors.empty() ) {
        errors.front().send( message_handler );
        errors.pop_front();
    }
}

void TIFFOperation::make_warning(
    const char *module, const char *fmt, va_list ap)
{
    assert( current );
    if (!suppress_TIFF_warnings) {
        char buffer[4096];
        vsnprintf( buffer, 4095, fmt, ap );
        simparm::Message m("Warning " + current->error_title,
            buffer, simparm::Message::Warning );
        m.send( current->message_handler );
    }
}

void TIFFOperation::make_error(
    const char *module, const char *fmt, va_list ap)
{
    // Work around the broken libtiff 4.x, which tags a warning as an error.
    if (strcmp("Incorrect count for \"%s\"; tag ignored", fmt) == 0) {
        make_warning(module, fmt, ap);
        return;
    }

    assert( current );
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
    simparm::Message m("Error " + current->error_title,
        buffer, simparm::Message::Error );
    current->errors.push_back( m );
}

void TIFFOperation::ignore(
    const char *, const char * fmt, va_list ap) 
{
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
}

void TIFFOperation::throw_exception_for_errors()
{
    if ( ! errors.empty() ) {
        std::runtime_error error( errors.front().message );
        errors.clear();
        throw error;
    }
}

}
