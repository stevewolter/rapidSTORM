#define DSTORM_CARCONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"
#include "Grand.h"
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/SourceFactory.h>
#include <dStorm/output/Basename.h>
#include <dStorm/Engine.h>

#include <dStorm/helpers/thread.h>
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

using namespace std;

namespace dStorm {

class GrandConfig::TreeRoot : public simparm::Object, public output::FilterSource
{
    output::Config* my_config;
    output::Capabilities cap;

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

    void set_trace_capability( const input::Traits<output::LocalizedImage>& t ) {
        cap.set_source_image( t.source_image_is_set );
        cap.set_smoothed_image( t.smoothed_image_is_set );
        cap.set_candidate_tree( t.candidate_tree_is_set );
        cap.set_cluster_sources( ! t.source_traits.empty() );
        this->set_source_capabilities( cap );
        
    }
};

GrandConfig::TreeRoot::TreeRoot()
: simparm::Object("EngineOutput", "dSTORM engine output"),
  output::FilterSource( static_cast<simparm::Object&>(*this) ),
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

GrandConfig::GrandConfig() 
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

GrandConfig::GrandConfig(const GrandConfig &c) 
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

GrandConfig::~GrandConfig() {
    ost::MutexLock lock( input::global_mutex() );
    outputRoot.reset( NULL );
}

void GrandConfig::create_input( std::auto_ptr<input::Link> p ) {
    input_listener = p->notify( boost::bind(&GrandConfig::traits_changed, this, _1) );
    input = p;
}

void GrandConfig::registerNamedEntries( simparm::Node& at ) {
   DEBUG("Registering named entries of CarConfig with " << size() << " elements before registering");
   outputBox.push_back( *outputRoot );
   input->registerNamedEntries(car_config);
   car_config.push_back( pistonCount );
   car_config.push_back( outputBox );
   car_config.push_back( configTarget );
   car_config.push_back( auto_terminate );
   at.push_back( car_config );
   DEBUG("Registered named entries of CarConfig with " << size() << " elements after registering");
}

void GrandConfig::add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> finder) {
    input->publish_meta_info();
    input->current_meta_info()->get_signal< signals::UseSpotFinder >()( *finder );
}

void GrandConfig::add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> fitter) {
    input->publish_meta_info();
    input->current_meta_info()->get_signal< signals::UseSpotFitter >()( *fitter );
}

void GrandConfig::add_input( std::auto_ptr<input::Link> l, InsertionPlace p) {
    if ( input.get() )
        input->insert_new_node( l, p );
    else
        create_input( l );
}

void GrandConfig::add_output( std::auto_ptr<output::OutputSource> o ) {
    outputConfig.addChoice( o.release() );
}

std::auto_ptr<input::BaseSource> GrandConfig::makeSource() {
    return std::auto_ptr<input::BaseSource>( input->makeSource() );
}

const input::MetaInfo&
GrandConfig::get_meta_info() const {
    return *input->current_meta_info();
}

void GrandConfig::traits_changed( boost::shared_ptr<const input::MetaInfo> traits ) {
    DEBUG("Basename declared in traits is " << traits->suggested_output_basename );
    outputRoot->set_output_file_basename( traits->suggested_output_basename );
    if ( traits->provides<output::LocalizedImage>() ) 
        outputRoot->set_trace_capability( *traits->traits<output::LocalizedImage>() );
}

void GrandConfig::all_modules_loaded() {
    input->publish_meta_info();
}

}
