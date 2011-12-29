#include "LibraryHandle.h"

namespace dStorm {

template <typename FunctionType>
void SafelyLoadedFunction<FunctionType>::load( 
    const lt_dlhandle& handle 
) {
    functionPtr = (FunctionType)
            lt_dlsym( handle, symbol_name );
    if ( functionPtr == NULL )
        throw std::runtime_error( "Plugin contains no " + std::string(symbol_name) 
                                    + " function." );
}

void LibraryHandle::init() {
    if ( handle == NULL )
        throw std::runtime_error( lt_dlerror() );
    try {
        desc.load( handle );
        config.load( handle );
        display_driver.load( handle );
    } catch (const std::exception& e) {
        throw std::runtime_error( "Invalid RapidSTORM plugin "
            + file + ": " + std::string(e.what()) );
    }
}

LibraryHandle::LibraryHandle( const char *file )
    : file(file), handle( lt_dlopenext(file) ),
        config("rapidSTORM_Config_Augmenter"),
        display_driver("rapidSTORM_Display_Driver"),
        desc("rapidSTORM_Plugin_Desc")
{ 
    init();
}

LibraryHandle::LibraryHandle( const LibraryHandle& other )
: file(other.file), handle( lt_dlopenext( file.c_str() ) ),
    config(other.config), display_driver(other.display_driver),
    desc(other.desc)
{
    init();
}

LibraryHandle::~LibraryHandle()
    {
        if ( handle != NULL )
            lt_dlclose( handle );
    }

void LibraryHandle::replace_display(
    std::auto_ptr<display::Manager>& driver 
) {
    display::Manager *d = driver.release();
    try {
        d = (*display_driver)( d );
    } catch (...) {
        driver.reset( d );
        throw;
    }
    driver.reset( d );
}

}
