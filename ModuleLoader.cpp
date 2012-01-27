#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include "ModuleLoader.h"
#include <boost/ptr_container/ptr_list.hpp>

#include "inputs/inputs.h"
#include "spotFinders/spotFinders.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include "outputs/BasicTransmissions.h"
#include <dStorm/Config.h>
#include "engine/ChainLink_decl.h"
#include "engine_stm/ChainLink.h"
#include "noop_engine/ChainLink_decl.h"
#include "guf/fitter.h"

#include <dStorm/display/Manager.h>
#include "wxDisplay/fwd.h"
#include "LibraryHandle.h"
#include "test-plugin/plugin.h"
#include "locprec/plugin.h"
#include "AndorCamera/plugin.h"
#include "viewer/plugin.h"

#include "debug.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {

struct LibLT_Handler {
    LibLT_Handler() { lt_dlinit(); }
    ~LibLT_Handler() { lt_dlexit(); }
};

struct ModuleLoader::Pimpl
{
    LibLT_Handler libLT_Handler;
    typedef boost::ptr_list<LibraryHandle> List;
    List lib_handles;
    std::set<std::string> loaded;

    std::auto_ptr<display::Manager> display;

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
: display( dStorm::display::make_wx_manager() )
{
    std::auto_ptr< display::Manager > tmp = display;
    display.reset( test::make_display( tmp.release() ) );
    load_plugins();
}

ModuleLoader::Pimpl::~Pimpl() {}

ModuleLoader::Pimpl::LoadResult
ModuleLoader::Pimpl::try_loading_module( const char *filename ) {
    try {
        std::auto_ptr<LibraryHandle> handle
            ( new LibraryHandle( filename ) );
        for (List::const_iterator i = lib_handles.begin(); 
             i !=lib_handles.end(); i++)
            if ( *i == *handle ) {
                DEBUG(filename  << " is a duplicate hit");
                return Failure;
            }
        DEBUG("Requesting plugin display driver from " 
              << filename);
        DEBUG("Display driver is before " << display.get() );
        handle->replace_display(display);
        DEBUG("Display dirver is now " << display.get() );
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
    std::string compiled_plugin_dir = RAPIDSTORM_PLUGIN_DIR;
    const char *plugin_dir = compiled_plugin_dir.c_str();
    char *env_plugin_dir = getenv("RAPIDSTORM_PLUGINDIR");
    if ( env_plugin_dir != NULL )
        plugin_dir = env_plugin_dir;

    DEBUG("Finding plugins in " << plugin_dir);
    lt_dlforeachfile( plugin_dir, lt_dlforeachfile_callback, this );
    DEBUG("Found plugins");

    display::Manager::setSingleton( *display );
}

void ModuleLoader::add_modules
    ( dStorm::Config& car_config )
{
    DEBUG("Adding basic input modules");
    dStorm::basic_inputs( &car_config );
    DEBUG("Adding rapidSTORM engine");
    car_config.add_input( engine::make_rapidSTORM_engine_link(), AsEngine );
    car_config.add_input( engine_stm::make_STM_engine_link(), AsEngine );
    car_config.add_input( noop_engine::makeLink(), AsEngine );
    DEBUG("Adding basic spot finders");
    car_config.add_spot_finder( spotFinders::make_Spalttiefpass() );
    car_config.add_spot_finder( spotFinders::make_Median() );
    car_config.add_spot_finder( spotFinders::make_Erosion() );
    car_config.add_spot_finder( spotFinders::make_Gaussian() );
    DEBUG("Adding basic output modules");
    dStorm::viewer::augment_config( car_config );
    dStorm::output::basic_outputs( &car_config );

    guf::augment_config( car_config );
    AndorCamera::augment_config( car_config );
    locprec::augment_config( car_config );
    test::make_config( &car_config );

    DEBUG("Iterating plugins");
    for ( Pimpl::List::iterator i = pimpl->lib_handles.begin(); i != pimpl->lib_handles.end();
          i++)
    {
        DEBUG("Adding plugin's modules");
        (*i) ( &car_config );
    }
}

std::string ModuleLoader::makeProgramDescription() {
    return pimpl->makeProgramDescription();
}

std::string ModuleLoader::Pimpl::makeProgramDescription() {
    std::stringstream ss;
    ss << PACKAGE_STRING;
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
