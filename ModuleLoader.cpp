#include "ModuleLoader.h"
#include <boost/ptr_container/ptr_list.hpp>

#include "inputs/inputs.h"
#include "spotFinders/spotFinders.h"
#include "outputs/BasicTransmissions.h"
#include "engine/CarConfig.h"

#include <dStorm/helpers/DisplayManager.h>
#include "wxDisplay/wxManager.h"
#include "LibraryHandle.h"

#include "debug.h"

namespace dStorm {

struct LibLT_Handler {
    LibLT_Handler() { lt_dlinit(); }
    ~LibLT_Handler() { lt_dlexit(); }
};

struct ModuleLoader::Pimpl {
    LibLT_Handler libLT_Handler;
    typedef boost::ptr_list<LibraryHandle> List;
    List lib_handles;
    std::set<std::string> loaded;

    std::auto_ptr<Display::Manager> display;

    Pimpl();
    ~Pimpl();

    void load_plugins();
    enum LoadResult { Loaded, Failure };
    LoadResult try_loading_module( const char *filename );
    static int lt_dlforeachfile_callback( 
        const char *filename, void* data );
    std::string makeProgramDescription();
};

ModuleLoader::ModuleLoader()
: pimpl( new Pimpl() ) {}

ModuleLoader::Pimpl::Pimpl()
: display( new Display::wxManager() )
{
    load_plugins();
}

ModuleLoader::Pimpl::~Pimpl() {}

ModuleLoader::Pimpl::LoadResult
ModuleLoader::Pimpl::try_loading_module( const char *filename ) {
    try {
        std::auto_ptr<LibraryHandle> handle
            ( new LibraryHandle( filename ) );
        DEBUG("Requesting plugin display driver from " 
              << filename);
        handle->replace_display(display);
        lib_handles.push_back( handle.release() );
    } catch( const std::exception& e ) {
        std::cerr << "Unable to load plugin '" << filename << "': " <<
            e.what() << "\n";
        return Failure;
    }
    loaded.insert( std::string(filename) );
    return Loaded;
}

int ModuleLoader::Pimpl::lt_dlforeachfile_callback 
    ( const char *filename, void* data )
{
    DEBUG("Considering name " << filename);
    ModuleLoader::Pimpl &m = *(ModuleLoader::Pimpl*)data;
    if ( m.loaded.find( filename ) != m.loaded.end() )
        return 0;
    else {
        DEBUG("Trying to load plugin " << filename);
        m.try_loading_module( filename );
        return 0;
    }
}

void ModuleLoader::Pimpl::load_plugins()
{
    const char *plugin_dir = DSTORM_PLUGIN_DIR;
    char *env_plugin_dir = getenv("RAPIDSTORM_PLUGINDIR");
    if ( env_plugin_dir != NULL )
        plugin_dir = env_plugin_dir;

    DEBUG("Finding plugins");
    lt_dlforeachfile( plugin_dir, lt_dlforeachfile_callback, this );
    DEBUG("Found plugins");
}

void ModuleLoader::add_modules
    ( engine::CarConfig& car_config )
{
    DEBUG("Adding basic input modules");
    dStorm::basic_inputs( &car_config.inputConfig );
    DEBUG("Adding basic spot finders");
    dStorm::spotFinders::basic_spotFinders( car_config.engineConfig );
    DEBUG("Adding basic output modules");
    dStorm::output::basic_outputs( &car_config.outputConfig );

    DEBUG("Iterating plugins");
    for ( Pimpl::List::iterator i = pimpl->lib_handles.begin(); i != pimpl->lib_handles.end();
          i++)
    {
        DEBUG("Adding plugin's input modules");
        (*i) ( &car_config.inputConfig );
        DEBUG("Adding plugin's spot finders");
        (*i) ( &car_config.engineConfig );
        DEBUG("Adding plugin's output modules");
        (*i) ( &car_config.outputConfig );
    }
}

std::string ModuleLoader::makeProgramDescription() {
    return pimpl->makeProgramDescription();
}

std::string ModuleLoader::Pimpl::makeProgramDescription() {
    std::stringstream ss;
    for ( List::iterator i = lib_handles.begin();
                            i != lib_handles.end(); i++)
    {
        List::iterator end_test = i; ++end_test;
        if ( i == lib_handles.begin() )
            ss << " with ";
        else if ( end_test == lib_handles.end() )
            ss << " and ";
        else 
            ss << ", ";
        ss << i->getDesc();
    }
    return ss.str();
}

static ModuleLoader *ml = NULL;

void ModuleLoader::makeSingleton() 
    { ml = new ModuleLoader(); }
ModuleLoader& ModuleLoader::getSingleton() {
    if ( ml != NULL )
        return *ml; 
    else 
        throw std::logic_error("ModuleLoader used before "
                               "initialization");
}
void ModuleLoader::destroySingleton()
    { if ( ml != NULL ) { delete ml; ml = NULL; } }

}
