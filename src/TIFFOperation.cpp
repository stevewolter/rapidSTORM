#include "TIFFOperation.h"
#include <cassert>
#include <stdexcept>

namespace dStorm {

ost::Mutex TIFFOperation::mutex;
TIFFOperation* TIFFOperation::current = NULL;

TIFFOperation::TIFFOperation( 
    std::string error_title,
    bool suppress_warnings
) : lock(mutex),
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
        std::cerr << errors.front() << std::endl;
        errors.pop_front();
    }
}

void TIFFOperation::make_warning(
    const char *module, const char *fmt, va_list ap)
{
    assert( current );
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
    std::cerr << "Warning " << current->error_title << ": " << buffer << std::endl;
}

void TIFFOperation::make_error(
    const char *module, const char *fmt, va_list ap)
{
    assert( current );
    char buffer[4096];
    vsnprintf( buffer, 4095, fmt, ap );
    current->errors.push_back( "Error " + current->error_title + ": " + buffer );
}

void TIFFOperation::ignore(
    const char *, const char *, va_list) {}

void TIFFOperation::throw_exception_for_errors()
{
    if ( ! errors.empty() )
        throw std::runtime_error( errors.front() );
}

}
