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
        input.load( handle );
        engine.load( handle );
        output.load( handle );
        display_driver.load( handle );
        cleanup.load( handle );
    } catch (const std::exception& e) {
        throw std::runtime_error( "Invalid RapidSTORM plugin "
            + file + ": " + std::string(e.what()) );
    }
}

LibraryHandle::LibraryHandle( const char *file )
    : file(file), handle( lt_dlopenext(file) ),
        input("rapidSTORM_Input_Augmenter"),
        engine("rapidSTORM_Engine_Augmenter"),
        output("rapidSTORM_Output_Augmenter"),
        display_driver("rapidSTORM_Display_Driver"),
        desc("rapidSTORM_Plugin_Desc"),
        cleanup("rapidSTORM_Cleanup_Handler")
{ 
    init();
}

LibraryHandle::LibraryHandle( const LibraryHandle& other )
: file(other.file), handle( lt_dlopenext( file.c_str() ) ),
    input(other.input), engine(other.engine),
    output(other.output), display_driver(other.display_driver),
    desc(other.desc), cleanup(other.cleanup)
{
    init();
}

LibraryHandle::~LibraryHandle()
    {
        if ( handle != NULL )
            lt_dlclose( handle );
    }

void LibraryHandle::replace_display(
    std::auto_ptr<Display::Manager>& driver 
) {
    Display::Manager *d = driver.release();
    try {
        d = (*display_driver)( d );
    } catch (...) {
        driver.reset( d );
        throw;
    }
    driver.reset( d );
}

}
