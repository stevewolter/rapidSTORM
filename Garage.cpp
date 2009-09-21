#include "Garage.h"
#include "GarageConfig.h"
#include <dStorm/Car.h>
#include <dStorm/BasicOutputs.h>
#include <dStorm/FilterSource.h>
#ifdef INCLUDE_LOCPREC
#include <locprec/TransmissionSourceFactory.h>
#include <locprec/NoiseSource.h>
#endif
#include <memory>
#include "Logo.h"
#include <algorithm>
#include <functional>
#include "foreach.h"
#include <simparm/IO.hh>
#include <dStorm/transmissions/LocalizationList.h>
#include <fstream>
#include <stdlib.h>

using namespace dStorm;
using namespace CImgBuffer;
using namespace std;

char *get_dir_name(char *file) throw();

GarageConfig::GarageConfig(TransmissionSourceFactory* tc) throw()
: simparm::Set("dSTORM", PACKAGE_STRING),
  my_tc( (tc != NULL) ? NULL :
#ifdef INCLUDE_LOCPREC
    new locprec::Outputs()
#else
    new dStorm::BasicOutputs()
#endif
  ),
  carConfig((tc) ? *tc : *my_tc),
  externalControl("TwiddlerControl", "Enable stdin/out control interface"),
  showTransmissionTree("ShowTransmissionTree", 
                       "Output tree view of transmissions"),
  run("Run", "Run")
{
   STATUS("Constructing GarageConfig");
#ifdef INCLUDE_LOCPREC
   STATUS("Registering random noise source");
   carConfig.inputConfig.inputMethod.addChoice(
        new locprec::NoiseConfig<unsigned short>( carConfig.inputConfig ));
#endif

   PROGRESS("Building externalControl");
   externalControl = false;
   externalControl.setUserLevel(Entry::Expert);

   PROGRESS("Building run");
   run.setHelp("Whenever this trigger is triggered or the button "
               "clicked, the dStorm engine will be run with the "
               "current parameters.");
   run.setUserLevel(Entry::Beginner);

   carConfig.inputConfig.basename.addChangeCallback(*this);

    registerNamedEntries();
}

GarageConfig::GarageConfig(const GarageConfig &c) throw()
: simparm::Node(c), 
  simparm::Set(c),
  simparm::Node::Callback(),
  carConfig(c.carConfig),
  externalControl(c.externalControl),
  showTransmissionTree(c.showTransmissionTree),
  run(c.run)
{
   carConfig.inputConfig.basename.addChangeCallback(*this);
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
    if ( &src == &carConfig.inputConfig.basename &&
                cause == ValueChanged && externalControl() ) 
    {
        bool appended = false;
        dStorm::TransmissionSource::BasenameResult r;
        std::string basename = carConfig.inputConfig.basename();
        avoid_auto_filenames.clear();
        do {
            if ( carConfig.inputConfig.inputFile() != "" )
                avoid_auto_filenames
                    .insert( carConfig.inputConfig.inputFile() );

            r = carConfig.outputConfig.set_output_file_basename
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
        printTC( carConfig.outputConfig, 0 );
        exit(0);
    }
}


void GarageConfig::registerNamedEntries() throw() {
    push_back( carConfig );
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
: autoConfig( new GarageConfig() ),
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
        config.carConfig.inputConfig.inputFile = argv[arg];
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

#ifndef D3_INTERNAL_VERSION
        cimg_library::Logo logo;
#endif
        simparm::IO ioManager(&cin, &cout);
        PROGRESS("Setting package name");
        ioManager.setDesc(PACKAGE_STRING);
        ioManager.showTabbed = true;
        config.carConfig.push_back( config.run );
        ioManager.push_back( config.carConfig );
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
    Car *car = new Car(config.carConfig);
    car->detach();
}

void Garage::_drive()
{
    STATUS("Made new car for drive");
    Car(config.carConfig).drive();
}

#include <libgen.h>

char *get_dir_name(char *in) throw() {
    return dirname(in);
}
