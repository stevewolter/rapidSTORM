#define DSTORM_CARCONFIG_CPP
#define VERBOSE
#include "debug.h"
#include "Config.h"
#include <dStorm/input/Config.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/SourceFactory.h>
#include <dStorm/output/Basename.h>

#include <dStorm/helpers/thread.h>
#include <sstream>

#include <simparm/ChoiceEntry_Impl.hh>

#include <cassert>

#include <time.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <simparm/Menu.hh>
#include <simparm/URI.hh>
#include "doc/help/rapidstorm_help_file.h"

using namespace std;

namespace dStorm {

class Config::TreeRoot : public simparm::Object, public output::FilterSource
{
    output::Config* my_config;
  public:
    TreeRoot();
    TreeRoot( const TreeRoot& other )
    : simparm::Object(other),
      output::FilterSource( static_cast<simparm::Object&>(*this), other)
    {
        DEBUG("Copying output tree root");
        this->set_output_factory( *other.my_config );
        my_config = dynamic_cast<output::Config*>(getFactory());
    }
    ~TreeRoot() {
        DEBUG("Destroying output tree root");
    }

    TreeRoot* clone() const { return new TreeRoot(*this); }
    std::string getDesc() const { return desc(); }
    output::Config &root_factory() { return *my_config; }
};

Config::TreeRoot::TreeRoot()
: simparm::Object("EngineOutput", "dSTORM engine output"),
  output::FilterSource( static_cast<simparm::Object&>(*this) )
{
    DEBUG("Building output tree root node at " << 
            &static_cast<Node&>(*this) );
    {
        output::Config exemplar;
        DEBUG("Setting output factory from exemplar t  " << static_cast<output::SourceFactory*>(&exemplar) << " in config at " << &exemplar);
        this->set_output_factory( exemplar );
        DEBUG("Destructing exemplar config");
    }
    DEBUG("Setting source capabilities");
    this->set_source_capabilities( 
        output::Capabilities()
            .set_source_image()
            .set_smoothed_image()
            .set_candidate_tree()
            .set_input_buffer() );

    DEBUG("Downcasting own config handle");
    assert( getFactory() != NULL );
    my_config = dynamic_cast<output::Config*>(getFactory());
    assert( my_config != NULL );
    DEBUG("Finished building output tree node");
}

Config::Config() 
: Set("Car", "Job options"),
  simparm::Listener(simparm::Event::ValueChanged),
  _inputConfig( new input::Config() ),
  _engineConfig( new engine::Config() ),
  outputRoot( new TreeRoot() ),
  inputConfig(*_inputConfig),
  engineConfig(*_engineConfig),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  helpMenu( "HelpMenu", "Help" ),
  outputBox("Output", "Output options"),
  configTarget("SaveConfigFile", "Store config used in computation in"),
  auto_terminate("AutoTerminate", "Automatically terminate finished jobs",
                 true)
{
   configTarget.setUserLevel(simparm::Object::Intermediate);
   auto_terminate.setUserLevel(simparm::Object::Expert);

   helpMenu.push_back( std::auto_ptr<simparm::Node>(
        new simparm::URI("BugTracker", "Report a bug", 
            "http://www.physik.uni-bielefeld.de/all/flyspray/index.php?do=register" ) ),
            true );
   helpMenu.push_back( std::auto_ptr<simparm::Node>(
        new simparm::FileEntry("rapidSTORMManual", "rapid2storm manual", 
            std::string("share/doc/") + dStorm::HelpFileName ) ),
            true );
   DEBUG("Made menu items");

    registerNamedEntries();
}

Config::Config(const Config &c) 
: simparm::Set(c),
  simparm::Listener(simparm::Event::ValueChanged),
  _inputConfig(c.inputConfig.clone()),
  _engineConfig(c.engineConfig.clone()),
  outputRoot(c.outputRoot->clone()),
  inputConfig(*_inputConfig),
  engineConfig(*_engineConfig),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  helpMenu( c.helpMenu ),
  outputBox(c.outputBox),
  configTarget(c.configTarget),
  auto_terminate(c.auto_terminate)
{
    registerNamedEntries();
    DEBUG("Copied Car config, basename is now " << inputConfig.basename());
}

Config::~Config() {
    outputRoot.reset( NULL );
    _engineConfig.reset( NULL );
    _inputConfig.reset( NULL );
}

void Config::registerNamedEntries() {
   DEBUG("Registering named entries of CarConfig");
   receive_changes_from( inputConfig.basename.value );
   outputBox.push_back( *outputRoot );
   push_back( inputConfig );
   push_back( engineConfig );
   push_back( outputBox );
   push_back( configTarget );
   push_back( auto_terminate );
   push_back( helpMenu );
   DEBUG("Registered named entries of CarConfig");
}

void Config::operator()(const simparm::Event&)
{
    output::Basename bn( inputConfig.basename() );
    
    DEBUG("Got new basename " << bn.unformatted()() << " for config " << this);
    outputSource.set_output_file_basename( bn );
}

}
