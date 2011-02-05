#ifndef DSTORM_LIBRARY_HANDLE_H
#define DSTORM_LIBRARY_HANDLE_H

#include <ltdl.h>
#include <string>
#include <dStorm/ModuleInterface.h>

namespace dStorm {

template <typename FunctionType>
struct SafelyLoadedFunction {
    const char *symbol_name;
    FunctionType functionPtr;
  public:
    SafelyLoadedFunction(const char *symbol_name)
        : symbol_name(symbol_name), functionPtr(NULL) {}
    void load( const lt_dlhandle& handle );

    FunctionType operator*() { return functionPtr; }
};

class LibraryHandle {
    std::string file;
    const lt_dlhandle handle;

    SafelyLoadedFunction<RapidSTORM_Config_Augmenter> config;
    SafelyLoadedFunction<RapidSTORM_Display_Driver> display_driver;
    SafelyLoadedFunction<RapidSTORM_Plugin_Desc> desc;

    void init();
  public:
    LibraryHandle( const char *file );
    LibraryHandle( const LibraryHandle& other );
    
    LibraryHandle *clone() 
        { return new LibraryHandle(*this); }

    ~LibraryHandle();

    void operator()( dStorm::Config* config ) 
       { (*this->config)( config ); }

    inline bool operator==(const LibraryHandle& l) const
        { return l.handle == handle; }
    inline bool operator!=(const LibraryHandle& l) const
        { return l.handle != handle; }

    void replace_display( std::auto_ptr<Display::Manager>& );

    const char *getDesc() { return (*desc)(); }
};

}

#endif
