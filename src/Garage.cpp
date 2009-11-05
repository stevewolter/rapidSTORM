#include "Garage.h"
#include "GarageConfig.h"
#include <engine/Car.h>
#include <dStorm/output/BasicOutputs.h>
#include <dStorm/output/FilterSource.h>
#include <memory>
#include <algorithm>
#include <functional>
#include <simparm/IO.hh>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <sys/types.h>

#include <ltdl.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/ModuleInterface.h>

#include "inputs/inputs.h"
#include "spotFinders/spotFinders.h"
#include "outputs/BasicTransmissions.h"
#include "doc/help/rapidstorm_help_file.h"

using namespace dStorm;
using namespace CImgBuffer;
using namespace std;

char *get_dir_name(char *file) throw();

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
    SafelyLoadedFunction<RapidSTORM_Plugin_Desc> desc;

    void init() {
        if ( handle == NULL )
            throw std::runtime_error( "Unable to open " + string(file) );
        try {
            desc.load( handle );
            input.load( handle );
            engine.load( handle );
            output.load( handle );
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
          desc("rapidSTORM_Plugin_Desc")
    { 
        init();
    }
    LibraryHandle( const LibraryHandle& other )
    : file(other.file), handle( lt_dlopenext( file.c_str() ) ),
      input(other.input), engine(other.engine),
      output(other.output), desc(other.desc)
    {
        init();
    }
    
    LibraryHandle *clone() { return new LibraryHandle(*this); }

    ~LibraryHandle() 
    {
        if ( handle != NULL )
            lt_dlclose( handle );
    }

    void operator()( CImgBuffer::Config* inputs ) {
        (*input)( inputs );
    }
    void operator()( dStorm::Config* engine_config ) {
        (*engine)( engine_config );
    }
    void operator()( dStorm::BasicOutputs* outputs ) {
        (*output)( outputs );
    }
    const char *getDesc() { return (*desc)(); }
};

ModuleHandler::LoadResult
ModuleHandler::try_loading_module( const char *filename ) {
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
    return ModuleHandler::Loaded;
}

int ModuleHandler::lt_dlforeachfile_callback 
    ( const char *filename, void* data )
{
    ModuleHandler &m = *(ModuleHandler*)data;
    if ( m.loaded.find( filename ) != m.loaded.end() )
        return 0;
    else {
        LoadResult result = m.try_loading_module( filename );
        return (( result == Loaded ) ? 1 : 0);
    }
}

ModuleHandler::ModuleHandler()
{
    lt_dlinit();

    const char *plugin_dir = DSTORM_PLUGIN_DIR;
    char *env_plugin_dir = getenv("RAPIDSTORM_PLUGINDIR");
    if ( env_plugin_dir != NULL )
        plugin_dir = env_plugin_dir;

    STATUS("Finding plugins");
    lt_dlforeachfile( plugin_dir, lt_dlforeachfile_callback, this );
    STATUS("Found plugins");
}

ModuleHandler::ModuleHandler( const ModuleHandler& o )
: lib_handles( o.lib_handles)
{
    lt_dlinit();
}

ModuleHandler::~ModuleHandler() {
    lib_handles.clear();
    lt_dlexit();
}

void ModuleHandler::add_input_and_engine_modules
    ( dStorm::CarConfig& car_config )
{
    for ( List::iterator i = lib_handles.begin(); i != lib_handles.end();
          i++)
    {
        (*i) ( &car_config.inputConfig );
        (*i) ( &car_config.engineConfig );
    }
}

void ModuleHandler::add_output_modules
    ( dStorm::BasicOutputs& tcf )
{
    for ( List::iterator i = lib_handles.begin();
                            i != lib_handles.end(); i++)
        (*i) ( &tcf );
}

std::string ModuleHandler::getDesc() {
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

GarageConfig::GarageConfig(ModuleHandler& module_handler) throw()
: simparm::Set("dSTORM", std::string(PACKAGE_STRING)
    + module_handler.getDesc()),
  externalControl("TwiddlerControl", "Enable stdin/out control interface"),
  showTransmissionTree("ShowTransmissionTree", 
                       "Output tree view of transmissions"),
  run("Run", "Run")
{
   tcf.reset( new dStorm::BasicOutputs() );
   dStorm::basic_outputs( tcf.get() );
   module_handler.add_output_modules( *tcf );
   carConfig.reset( new dStorm::CarConfig( *tcf ) );
   dStorm::basic_inputs( &carConfig->inputConfig );
   dStorm::basic_spotFinders( carConfig->engineConfig );
   STATUS("Constructing GarageConfig");
   module_handler.add_input_and_engine_modules( *carConfig );

   PROGRESS("Building externalControl");
   externalControl = false;
   externalControl.setUserLevel(Entry::Expert);

   PROGRESS("Building run");
   run.setHelp("Whenever this trigger is triggered or the button "
               "clicked, the dStorm engine will be run with the "
               "current parameters.");
   run.setUserLevel(Entry::Beginner);

   carConfig->inputConfig.basename.addChangeCallback(*this);

    registerNamedEntries();
}

GarageConfig::GarageConfig(const GarageConfig &c) throw()
: simparm::Node(c), 
  simparm::Set(c),
  simparm::Node::Callback(),
  carConfig(c.carConfig->clone()),
  externalControl(c.externalControl),
  showTransmissionTree(c.showTransmissionTree),
  run(c.run)
{
   carConfig->inputConfig.basename.addChangeCallback(*this);
   registerNamedEntries();
}

static void printTC( const OutputSource& src, int indent ) {
    if ( indent < 2 )
        cout << ((indent)?" ":src.getName()) << "\n";
    else {
        cout << std::string(indent-2, ' ') << "|-" << src.getName() << "\n";
    }
    const FilterSource* fwd = dynamic_cast<const FilterSource*>(&src);
    if (fwd != NULL) {
        for (FilterSource::const_iterator
                            i = fwd->begin(); i != fwd->end(); i++)
            printTC(**i, indent+2);
    }
}

void GarageConfig::operator()(Node& src, Cause cause, Node *) throw() {
    if ( &src == &carConfig->inputConfig.basename &&
                cause == ValueChanged && externalControl() ) 
    {
        bool appended = false;
        dStorm::TransmissionSource::BasenameResult r;
        std::string basename = carConfig->inputConfig.basename();
        avoid_auto_filenames.clear();
        do {
            if ( carConfig->inputConfig.inputFile() != "" )
                avoid_auto_filenames
                    .insert( carConfig->inputConfig.inputFile() );

            r = carConfig->outputConfig.set_output_file_basename
                    ( basename, avoid_auto_filenames );
            if ( !appended ) 
                basename += 'a'; 
            else 
                basename[ basename.size()-1 ]++;
            appended = true;
        } while ( r == dStorm::TransmissionSource::Basename_Conflicted );
    } else if ( &src == &showTransmissionTree && 
                showTransmissionTree.triggered() )
    {
        showTransmissionTree.untrigger();
        printTC( carConfig->outputConfig, 0 );
        exit(0);
    }
}


void GarageConfig::registerNamedEntries() throw() {
    push_back( *carConfig );
    push_back( externalControl );
    push_back( showTransmissionTree );

    receive_changes_from( showTransmissionTree );
}

static bool load_file(const std::string& name, simparm::Node& node) {
    std::ifstream config_file( name.c_str() );
    if ( !config_file )
        return false;
    else {
        while ( config_file )
            node.processCommand( config_file );
        return true;
    }
}

Garage::Garage(int argc, char *argv[]) 
: moduleHandler( new ModuleHandler() ),
  autoConfig( new GarageConfig(*moduleHandler) ),
  config( *autoConfig )
{
    STATUS("Garage argument constructor called");

    STATUS("Reading default config file");
    const char *home = getenv("HOME"),
               *homedrive = getenv("HOMEDRIVE"),
               *homepath = getenv("HOMEPATH");
    bool have_file = false;
    if ( !have_file && argc > 3 && string(argv[1]) == "--config" ) {
        have_file = load_file(argv[2], config);
        if ( have_file ) {
            argc -= 2;
            argv += 2;
        }
    }
    if ( !have_file && home != NULL )
        have_file = load_file( std::string(home) + "/.dstorm", config );
    if ( !have_file && homedrive != NULL && homepath != NULL )
        have_file = load_file( 
            std::string(homedrive) + std::string(homepath) + "/dstorm.txt",
            config );
    if ( !have_file )
        load_file( 
            std::string( get_dir_name(argv[0]) )+"/dstorm-config.txt",
            config );

    STATUS("Reading command line arguments");
    int first_nonoption = 0;
    if (argc > 0) {
        first_nonoption = config.readConfig(argc, argv);
        if ( first_nonoption == -1 )
            exit(1);
    }

    STATUS("Running dSTORM");
    for (int arg = first_nonoption; arg < argc; arg++) {
        config.carConfig->inputConfig.inputFile = argv[arg];
        if ( arg < argc - 1)
            config.run.trigger();
    }
    init();

    if ( ! config.externalControl() ) {
        _drive();
        autoConfig.reset(NULL);
    }
}

Garage::Garage(GarageConfig& config) throw()
: config(config)
{
    STATUS("Garage config constructor called");
    init();
}

void set_all_ConfigNodes_to(simparm::Node &node, string nodeName, string to)
{
    if ( node.getName() == nodeName ) {
        stringstream ss(to);
        node.processCommand( ss );
    }
    
    for (simparm::Node::const_iterator i = node.begin(); i != node.end(); i++)
        set_all_ConfigNodes_to( **i, nodeName, to );
}

void Garage::init() throw() {
    STATUS("Connecting Garage to event handler");
    config.run.value.addChangeCallback(*this);

    STATUS("Checking for external control request");
    if (config.externalControl()) {
        //((BasicTransmissions*)factory.get())->addFilterToDefaultConfig();

        simparm::Attribute<std::string> help_file
            ("help_file", dStorm::HelpFileName);
        simparm::IO ioManager(&cin, &cout);
        PROGRESS("Setting package name");
        ioManager.setDesc(config.getDesc());
        ioManager.push_back( help_file );
        ioManager.showTabbed = true;
        config.carConfig->push_back( config.run );
        ioManager.push_back( *config.carConfig );
        ioManager.processInput();

        Car::terminate_all_Car_threads();
        autoConfig.reset( NULL );
    } else {
        //set_all_ConfigNodes_to(config, "ShowOutput", " = false");
    }
}

Garage::~Garage() throw() {
    STATUS("Destructing " << (void*)this);
}

void Garage::operator()(Node& src, Cause, Node *) throw() {
    if (&src == &config.run.value && config.run.triggered()) {
        config.run.untrigger();
        STATUS("Made new car for event");
        try {
            cruise();
        } catch (const std::exception& e) {
            cerr << PACKAGE_NAME << ": " << e.what() << endl;
        }
    }
}

void Garage::cruise() throw(std::exception)
{
    STATUS("Made new car for cruise");
    Car *car = new Car(*config.carConfig);
    car->detach();
}

void Garage::_drive()
{
    STATUS("Made new car for drive");
    Car(*config.carConfig).drive();
}

#include <libgen.h>

char *get_dir_name(char *in) throw() {
    return dirname(in);
}
