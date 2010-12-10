#include "Garage.h"
#include "GarageConfig.h"
#include <dStorm/Car.h>
#include <dStorm/BasicTransmissions.h>
#ifdef INCLUDE_LOCPREC
#include <locprec/RegionSegmenterConfig.h>
#endif
#include <memory>
#include "Logo.h"
#include <algorithm>
#include <functional>
#include "foreach.h"
#include <ConfigIO.hh>
#include <dStorm/LocalizationList.h>

using namespace dStorm;
using namespace CImgBuffer;
using namespace std;

GarageConfig::GarageConfig(TransmissionSourceFactory& tc) throw()
: ConfigObject("MainConfig", PACKAGE_STRING),
  dStorm::CarConfig(tc),
#ifdef INCLUDE_LOCPREC
  useLocprec("ShowLocprec", "Show localisation precision options"),
#endif
  externalControl("TwiddlerControl", "Enable stdin/out control interface"),
  run("Run", "Run")
{
   STATUS("Constructing GarageConfig");
   PROGRESS("Setting package name");

#ifdef INCLUDE_LOCPREC
   useLocprec.setUserLevel(ConfigEntry::Beginner);
#endif

   PROGRESS("Building externalControl");
   externalControl = false;
   externalControl.setUserLevel(ConfigEntry::Expert);

   PROGRESS("Building run");
   run.setHelp("Whenever this trigger is triggered or the button "
               "clicked, the dStorm engine will be run with the "
               "current parameters.");
   run.setUserLevel(ConfigEntry::Beginner);

   inputConfig.inputFile.addChangeCallback(*this);
#ifdef INCLUDE_LOCPREC
    useLocprec.addChangeCallback(*this);
    (*this)( useLocprec );
#endif

    registerNamedEntries();
}

GarageConfig::GarageConfig(const GarageConfig &c) throw()
: ConfigObject(c),
  simparm::ConfigNode::Callback(),
  dStorm::CarConfig(c),
#ifdef INCLUDE_LOCPREC
  useLocprec(c.useLocprec),
#endif
  externalControl(c.externalControl),
  run(c.run)
{
   inputConfig.inputFile.addChangeCallback(*this);
#ifdef INCLUDE_LOCPREC
   useLocprec.addChangeCallback(*this);
   gotEvent( useLocprec );
#endif
   registerNamedEntries();
}

void GarageConfig::operator()(ConfigNode& src, Cause, ConfigNode *) throw() {
    if ( &src == &inputConfig.inputFile && externalControl() ) {
        string replacement = inputConfig.inputFile();
        int dotpos = replacement.rfind( '.' );
        string basename = replacement.substr( 0, dotpos );
        // TODO if ( (basename + ".txt") != replacement )
        // TODO    outputFile = basename + ".txt";
        // TODO tifFile = basename + ".png";
#ifdef INCLUDE_LOCPREC
    } else if ( &src == &useLocprec ) {
        ConfigSet locprecOpts;
        locprec::RegionSegmenterConfig::registerEntries(locprecOpts);
        foreach_const( option, vector<ConfigEntry*>,
                       locprecOpts.getEntries() )
            (*option)->setViewable( useLocprec() );
        
#endif
    }
}


void GarageConfig::registerNamedEntries() throw() {
#ifdef INCLUDE_LOCPREC
   s.register_entry(&useLocprec);
   locprec::RegionSegmenterConfig::registerEntries(s);
#endif
   register_entry(&externalControl);
   register_entry(&run);
}

Garage::Garage(int argc, char *argv[]) throw() 
: factory(
#ifdef INCLUDE_LOCPREC
    new TODO
#else
    new dStorm::BasicTransmissions()
#endif
),
  autoConfig( new GarageConfig(*factory) ),
  config( *autoConfig )
{
    STATUS("Garage argument constructor called");
    autoConfig->readConfig(argc, argv);

    init();
    if ( ! config.externalControl() ) {
        try {
            cruise();
        } catch (const std::exception &e) {
            cerr << PACKAGE_NAME << ": " << e.what() << endl;
        }
    }
}

Garage::Garage(GarageConfig& config) throw()
: config(config)
{
    STATUS("Garage config constructor called");
    init();
}

void Garage::init() throw() {
    STATUS("Connecting Garage to event handler");
    config.run.addChangeCallback(*this);

    STATUS("Checking for external control request");
    if (config.externalControl()) {
        config.externalControl.setViewable(false);
        config.stderr_progress.setViewable(false);
        // TODO config.showOutput = true;
        config.stderr_progress = false;

#ifndef D3_INTERNAL_VERSION
        cimg_library::Logo logo;
#endif
        simparm::ConfigIO ioManager(&cin, &cout);
        ioManager.push_back( config );
        ioManager.processInput();
    }
}

Garage::~Garage() throw() {
    STATUS("Destructing " << (void*)this);
}

void Garage::operator()(ConfigNode& src, Cause, ConfigNode *) throw() {
    if (&src == &config.run && config.run.triggered()) {
        config.run.untrigger();
        STATUS("Made new car for event");
        try {
            cruise();
        } catch (const std::exception& e) {
            cerr << PACKAGE_NAME << ": " << e.what() << endl;
        }
    }
}

auto_ptr<Localizations> Garage::drive() 
    throw(std::exception) 
{
    std::auto_ptr<LocalizationList> locList( new LocalizationList() );
    _drive(NULL, locList.get());
    return locList->releaseResults();
}

void Garage::cruise() throw(std::exception)
{
    STATUS("Made new car for cruise");
    Car *car = new Car(config);
#ifdef INCLUDE_LOCPREC
    config.locprec::RegionSegmenterConfig::addPassengers(*car);
#endif
    car->detach();
}

void Garage::_drive(Carburettor *in, Transmission *out)
    throw(std::exception) 
{
    STATUS("Made new car for drive");
    Car car(config, in, out);
#ifdef INCLUDE_LOCPREC
    config.RegionSegmenterConfig::addPassengers(car);
#endif
    car.run();
}
