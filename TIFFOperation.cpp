#include "TIFFOperation.h"
#include <simparm/Message.hh>
#include <dStorm/helpers/exception.h>
#include <cassert>

namespace dStorm {

ost::Mutex TIFFOperation::mutex;
TIFFOperation* TIFFOperation::current = NULL;

TIFFOperation::TIFFOperation( 
    std::string error_title,
    simparm::Node& message_handler, 
    bool suppress_warnings
) : lock(mutex),
    message_handler(message_handler) ,
    error_title(error_title)
{
    current = this;
    old_warning_handler = TIFFSetWarningHandler(
        (suppress_warnings) ? ignore : make_warning );
    old_error_handler = TIFFSetErrorHandler( make_error );
}

TIFFOperation::~TIFFOperation() {
    TIFFSetWarningHandler( old_warning_handler );
    TIFFSetErrorHandler( old_error_handler );
    current = NULL;
    while ( ! errors.empty() ) {
        message_handler.send( errors.front() );
        errors.pop_front();
    }
}

void TIFFOperation::make_warning(
    const char *module, const char *fmt, va_list ap)
{
    assert( current );
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
    simparm::Message m("Warning " + current->error_title,
        buffer, simparm::Message::Warning );
    current->message_handler.send( m );
}

void TIFFOperation::make_error(
    const char *module, const char *fmt, va_list ap)
{
    assert( current );
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
    simparm::Message m("Error " + current->error_title,
        buffer, simparm::Message::Error );
    current->errors.push_back( m );
}

void TIFFOperation::ignore(
    const char *, const char *, va_list) {}

void TIFFOperation::throw_exception_for_errors()
{
    if ( ! errors.empty() ) {
        dStorm::runtime_error_message error( errors.front() );
        errors.clear();
        throw error;
    }
}

}
