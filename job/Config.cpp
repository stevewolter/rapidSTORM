#define DSTORM_CARCONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"
#include "Config.h"
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/SourceFactory.h>
#include <dStorm/output/Basename.h>
#include <dStorm/Engine.h>

#include <sstream>

#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/InputMutex.h>

#include <dStorm/input/Forwarder.h>

#include <cassert>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <simparm/Menu.hh>
#include <simparm/URI.hh>

#include <dStorm/signals/UseSpotFinder.h>
#include <dStorm/signals/UseSpotFitter.h>

#include "Car.h"

using namespace std;

namespace dStorm {
namespace job {

class Config::TreeRoot : public simparm::Object, public output::FilterSource
{
    output::Config* my_config;
    output::Capabilities cap;

    std::string getName() const { return simparm::Object::getName(); }
    std::string getDesc() const { return simparm::Object::desc(); }
    void attach_ui( simparm::Node& ) { throw std::logic_error("Not implemented on tree base"); }

  public:
    TreeRoot();
    TreeRoot( const TreeRoot& other )
    : simparm::Object(other),
      output::FilterSource( other)
    {
        DEBUG("Copying output tree root");
        this->set_output_factory( *other.my_config );
        my_config = dynamic_cast<output::Config*>(getFactory());
    }
    ~TreeRoot() {
        DEBUG("Destroying output tree root");
    }

    TreeRoot* clone() const { return new TreeRoot(*this); }
    output::Config &root_factory() { return *my_config; }

    void set_trace_capability( const input::Traits<output::LocalizedImage>& t ) {
        cap.set_source_image( t.source_image_is_set );
        cap.set_smoothed_image( t.smoothed_image_is_set );
        cap.set_candidate_tree( t.candidate_tree_is_set );
        cap.set_cluster_sources( ! t.source_traits.empty() );
        this->set_source_capabilities( cap );
    }

    void attach_full_ui( simparm::Node& at ) { 
        simparm::NodeRef r = simparm::Object::attach_ui(at);
        attach_source_ui( r ); 
    }
};

Config::TreeRoot::TreeRoot()
: simparm::Object("EngineOutput", "dSTORM engine output"),
  output::FilterSource(),
  cap( output::Capabilities()
            .set_source_image()
            .set_smoothed_image()
            .set_candidate_tree()
            .set_input_buffer() )
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
    this->set_source_capabilities( cap );

    DEBUG("Downcasting own config handle");
    assert( getFactory() != NULL );
    my_config = dynamic_cast<output::Config*>(getFactory());
    assert( my_config != NULL );
    DEBUG("Finished building output tree node");
}

Config::Config() 
: car_config("Car", "Job options"),
  outputRoot( new TreeRoot() ),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  helpMenu( "HelpMenu", "Help" ),
  outputBox("Output", "Output options"),
  configTarget("SaveConfigFile", "Store config used in computation in"),
  auto_terminate("AutoTerminate", "Automatically terminate finished jobs",
                 true),
  pistonCount("CPUNumber", "Number of CPUs to use")
{
   configTarget.setUserLevel(simparm::Object::Intermediate);
   auto_terminate.setUserLevel(simparm::Object::Expert);

    pistonCount.setUserLevel(simparm::Object::Expert);
    pistonCount.helpID = "#CPUNumber";
    pistonCount.setHelp("Use this many parallel threads to compute the "
                        "STORM result. If you notice a low CPU usage, "
                        "raise this value to the number of cores you "
                        "have.");
#if defined(_SC_NPROCESSORS_ONLN)
    int pn = sysconf(_SC_NPROCESSORS_ONLN);
    pistonCount = (pn == 0) ? 8 : pn;
#elif defined(HAVE_WINDOWS_H)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    pistonCount = info.dwNumberOfProcessors;
#else
    pistonCount.setUserLevel(Object::Beginner);
    pistonCount = 8;
#endif

   DEBUG("Made menu items");
}

Config::Config(const Config &c) 
: car_config(c.car_config),
  outputRoot(c.outputRoot->clone()),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  helpMenu( c.helpMenu ),
  outputBox(c.outputBox),
  configTarget(c.configTarget),
  auto_terminate(c.auto_terminate),
  pistonCount(c.pistonCount)
{
    if ( c.input.get() )
        create_input( std::auto_ptr<input::Link>(c.input->clone()) );
    input->publish_meta_info();
    DEBUG("Copied Car config");
}

Config::~Config() {
    dStorm::input::InputMutexGuard lock( input::global_mutex() );
    car_config.clearParents();
    car_config.clearChildren();
    outputRoot.reset( NULL );
    input_listener.reset();
    input.reset();
}

void Config::create_input( std::auto_ptr<input::Link> p ) {
    input_listener = p->notify( boost::bind(&Config::traits_changed, this, _1) );
    input = p;
}

void Config::attach_ui( simparm::Node& at ) {
   DEBUG("Registering named entries of CarConfig with " << size() << " elements before registering");
   outputRoot->attach_full_ui(outputBox);
   input->registerNamedEntries(car_config);
   car_config.push_back( pistonCount );
   car_config.push_back( outputBox );
   car_config.push_back( configTarget );
   car_config.push_back( auto_terminate );
   at.push_back( car_config );
   DEBUG("Registered named entries of CarConfig with " << size() << " elements after registering");
}

void Config::add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> finder) {
    input->publish_meta_info();
    input->current_meta_info()->get_signal< signals::UseSpotFinder >()( *finder );
}

void Config::add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> fitter) {
    input->publish_meta_info();
    input->current_meta_info()->get_signal< signals::UseSpotFitter >()( *fitter );
}

void Config::add_input( std::auto_ptr<input::Link> l, InsertionPlace p) {
    if ( input.get() )
        input->insert_new_node( l, p );
    else
        create_input( l );
}

void Config::add_output( std::auto_ptr<output::OutputSource> o ) {
    outputConfig.addChoice( o.release() );
}

std::auto_ptr<input::BaseSource> Config::makeSource() const {
    return input->make_source();
}

const input::MetaInfo&
Config::get_meta_info() const {
    return *input->current_meta_info();
}

void Config::traits_changed( boost::shared_ptr<const input::MetaInfo> traits ) {
    DEBUG("Basename declared in traits is " << traits->suggested_output_basename );
    outputRoot->set_output_file_basename( traits->suggested_output_basename );
    if ( traits->provides<output::LocalizedImage>() ) 
        outputRoot->set_trace_capability( *traits->traits<output::LocalizedImage>() );
}

void Config::all_modules_loaded() {
    input->publish_meta_info();
}

void Config::create_and_run( JobMaster& master ) 
{
    new Car( &master, *this );
}

}
}
