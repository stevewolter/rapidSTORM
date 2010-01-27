#include "MasterConfig.h"
#include <boost/utility.hpp>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <boost/ptr_container/ptr_list.hpp>
#include <simparm/IO.hh>
#include <ltdl.h>
#include <dStorm/ModuleInterface.h>

#include "inputs/inputs.h"
#include "spotFinders/spotFinders.h"
#include "outputs/BasicTransmissions.h"
#include "doc/help/rapidstorm_help_file.h"

#include <dStorm/helpers/DisplayManager.h>
#include "wxDisplay/wxManager.h"

#include "debug.h"

namespace dStorm {

MasterConfig::Ptr::Ptr( MasterConfig* mc )
: m (*mc) 
{
    boost::lock_guard<boost::mutex> lock( m.usage_count_mutex );
    m.usage_count = 1;
    m.may_be_referenced = true;
}

MasterConfig::Ptr::Ptr( const MasterConfig::Ptr& o )
: m(o.m) 
{
    boost::lock_guard<boost::mutex> lock( m.usage_count_mutex );
    if ( ! m.may_be_referenced )
        throw std::runtime_error
            ("This MasterConfig is exclusively owned.");
    m.usage_count++;
}

MasterConfig::Ptr::~Ptr() {
    boost::unique_lock<boost::mutex> lock( m.usage_count_mutex );
    m.usage_count--;
    if ( m.usage_count == 0 )  {
        lock.unlock();
        delete &m;
    } else if ( m.usage_count == 1 )
        m.usage_count_is_one.notify_all();
}

void MasterConfig::Ptr::wait_for_exclusive_ownership() {
    boost::unique_lock<boost::mutex> lock( m.usage_count_mutex );
    while ( m.usage_count > 1 ) {
        DEBUG("Waiting on usage count " << m.usage_count);
        m.usage_count_is_one.wait( lock );
        DEBUG("Got usage count " << m.usage_count);
    }
    m.may_be_referenced = false;
}

template <typename FunctionType>
struct SafelyLoadedFunction {
    const char *symbol_name;
    FunctionType functionPtr;
  public:
    SafelyLoadedFunction(const char *symbol_name)
        : symbol_name(symbol_name), functionPtr(NULL) {}
    void load( const lt_dlhandle& handle )
    {
        functionPtr = (FunctionType)
                lt_dlsym( handle, symbol_name );
        if ( functionPtr == NULL )
            throw std::runtime_error( "Plugin contains no " + std::string(symbol_name) 
                                      + " function." );
    }

    FunctionType operator*() { return functionPtr; }
};

class LibraryHandle {
    std::string file;
    const lt_dlhandle handle;

    SafelyLoadedFunction<RapidSTORM_Input_Augmenter> input;
    SafelyLoadedFunction<RapidSTORM_Engine_Augmenter> engine;
    SafelyLoadedFunction<RapidSTORM_Output_Augmenter> output;
    SafelyLoadedFunction<RapidSTORM_Display_Driver> display_driver;
    SafelyLoadedFunction<RapidSTORM_Plugin_Desc> desc;

    void init() {
        if ( handle == NULL )
            throw std::runtime_error( "Unable to open " + 
                                      std::string(file) );
        try {
            desc.load( handle );
            input.load( handle );
            engine.load( handle );
            output.load( handle );
            display_driver.load( handle );
        } catch (const std::exception& e) {
            throw std::runtime_error( "Invalid RapidSTORM plugin "
                + file + ": " + std::string(e.what()) );
        }
    }

  public:
    LibraryHandle( const char *file )
        : file(file), handle( lt_dlopenext(file) ),
          input("rapidSTORM_Input_Augmenter"),
          engine("rapidSTORM_Engine_Augmenter"),
          output("rapidSTORM_Output_Augmenter"),
          display_driver("rapidSTORM_Display_Driver"),
          desc("rapidSTORM_Plugin_Desc")
    { 
        init();
    }
    LibraryHandle( const LibraryHandle& other )
    : file(other.file), handle( lt_dlopenext( file.c_str() ) ),
      input(other.input), engine(other.engine),
      output(other.output), display_driver(other.display_driver),
      desc(other.desc)
    {
        init();
    }
    
    LibraryHandle *clone() { return new LibraryHandle(*this); }

    ~LibraryHandle() 
    {
        if ( handle != NULL )
            lt_dlclose( handle );
    }

    void operator()( dStorm::input::Config* inputs ) {
        (*input)( inputs );
    }
    void operator()( dStorm::engine::Config* engine_config ) {
        (*engine)( engine_config );
    }
    void operator()( dStorm::output::Config* outputs ) {
        (*output)( outputs );
    }

    void replace_display( std::auto_ptr<Display::Manager>& driver )
    {
        Display::Manager *d = driver.release();
        try {
            d = (*display_driver)( d );
        } catch (...) {
            driver.reset( d );
            throw;
        }
        driver.reset( d );
    }

    const char *getDesc() { return (*desc)(); }
};

class _MasterConfig 
    : boost::noncopyable,
      public MasterConfig,
      public simparm::IO
{
  private:
    boost::mutex mutex;
    boost::condition all_modules_unregistered;
    int registered_nodes;

    void load_plugins();

    typedef boost::ptr_list<LibraryHandle> List;
    List lib_handles;
    std::set<std::string> loaded;

    std::auto_ptr<Display::Manager> display;

    enum LoadResult { Loaded, Failure };
    LoadResult try_loading_module( const char *filename );
    static int lt_dlforeachfile_callback( 
        const char *filename, void* data );

    simparm::Attribute<std::string> help_file;

    std::string getDesc();

    std::auto_ptr<Display::Manager> make_display_driver();

  public:
    _MasterConfig();
    ~_MasterConfig();

    void thread_safely_register_node( simparm::Node& node );
    void thread_safely_erase_node( simparm::Node& node );
    void add_modules( engine::CarConfig& config );
    void read_input(std::istream&);
};

MasterConfig::Ptr 
MasterConfig::create() {
    return MasterConfig::Ptr(new _MasterConfig());
}

MasterConfig::~MasterConfig() {
    DEBUG("Destroying master config at " << this);
}

_MasterConfig::_MasterConfig() 
: simparm::IO(NULL, &std::cout),
  registered_nodes(0),
  help_file("help_file", dStorm::HelpFileName) 
{
    lt_dlinit();
    load_plugins();

    display = make_display_driver();

    Display::Manager::setSingleton(*display);

    this->showTabbed = true;
    push_back( help_file );
    this->simparm::IO::setDesc( getDesc() );
}

_MasterConfig::~_MasterConfig() {
    boost::lock_guard<boost::mutex> lock(mutex);
    while ( registered_nodes > 0 )
        all_modules_unregistered.wait( mutex );
        
    display.reset( NULL );

    lib_handles.clear();
    lt_dlexit();
}

void _MasterConfig::thread_safely_register_node( simparm::Node& node ) {
    boost::lock_guard<boost::mutex> lock(mutex);
    this->simparm::Node::push_back( node );
    registered_nodes++;
}

void _MasterConfig::thread_safely_erase_node( simparm::Node& node ) {
    boost::lock_guard<boost::mutex> lock(mutex);
    this->simparm::Node::erase( node );
    --registered_nodes;
    if ( registered_nodes == 0 )
        all_modules_unregistered.notify_all();
}

void _MasterConfig::read_input( std::istream& input ) {
    this->set_input_stream( &input );
    this->processInput();
}

_MasterConfig::LoadResult
_MasterConfig::try_loading_module( const char *filename ) {
    try {
        std::auto_ptr<LibraryHandle> handle
            ( new LibraryHandle( filename ) );
        lib_handles.push_back( handle.release() );
    } catch( const std::exception& e ) {
        std::cerr << "Unable to load plugin '" << filename << "': " <<
            e.what() << "\n";
        return Failure;
    }
    loaded.insert( std::string(filename) );
    return Loaded;
}

int _MasterConfig::lt_dlforeachfile_callback 
    ( const char *filename, void* data )
{
    DEBUG("Considering name " << filename);
    _MasterConfig &m = *(_MasterConfig*)data;
    if ( m.loaded.find( filename ) != m.loaded.end() )
        return 0;
    else {
        DEBUG("Trying to load plugin " << filename);
        m.try_loading_module( filename );
        return 0;
    }
}

void _MasterConfig::load_plugins()
{
    const char *plugin_dir = DSTORM_PLUGIN_DIR;
    char *env_plugin_dir = getenv("RAPIDSTORM_PLUGINDIR");
    if ( env_plugin_dir != NULL )
        plugin_dir = env_plugin_dir;

    DEBUG("Finding plugins");
    lt_dlforeachfile( plugin_dir, lt_dlforeachfile_callback, this );
    DEBUG("Found plugins");
}

std::auto_ptr<Display::Manager>
_MasterConfig::make_display_driver()
{
    std::auto_ptr<Display::Manager> rv;
    rv.reset(
        &Display::wxManager::getSingleton() );

    for ( List::iterator i = lib_handles.begin(); i != lib_handles.end();
          i++)
    {
        DEBUG("Requesting plugin display driver");
        i->replace_display(rv);
    }

    return rv;
}
void _MasterConfig::add_modules
    ( dStorm::engine::CarConfig& car_config )
{
    DEBUG("Adding basic input modules");
    dStorm::basic_inputs( &car_config.inputConfig );
    DEBUG("Adding basic spot finders");
    dStorm::spotFinders::basic_spotFinders( car_config.engineConfig );
    DEBUG("Adding basic output modules");
    dStorm::output::basic_outputs( &car_config.outputConfig );

    DEBUG("Iterating plugins");
    for ( List::iterator i = lib_handles.begin(); i != lib_handles.end();
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

std::string _MasterConfig::getDesc() {
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

}
