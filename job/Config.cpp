#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif

#include <fstream>
#include <cassert>
#include <sstream>

#include <boost/smart_ptr/make_shared.hpp>
#include <dStorm/Engine.h>
#include <dStorm/input/Forwarder.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/output/Basename.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/SourceFactory.h>
#include <dStorm/signals/UseSpotFinder.h>
#include <dStorm/signals/UseSpotFitter.h>
#include <simparm/Menu.h>
#include <simparm/text_stream/RootNode.h>
#include <simparm/TreeRoot.h>
#include <simparm/TreeEntry.h>
#include <ui/serialization/Node.h>

#include "Car.h"
#include "Config.h"
#include "debug.h"
#include "ModuleLoader.h"

using namespace std;

namespace dStorm {
namespace job {

Config::Config( bool localization_replay_mode ) 
: car_config("Car", "Job options"),
  outputRoot( new OutputTreeRoot() ),
  localization_replay_mode( localization_replay_mode ),
  outputBox("Output", "Output options"),
  configTarget("SaveConfigFile", "Job options file", "-settings.txt"),
  auto_terminate("AutoTerminate", "Automatically terminate finished jobs", false),
  pistonCount("CPUNumber", "Number of CPUs to use", 1)
{
    configTarget.set_user_level(simparm::Beginner);
    auto_terminate.set_user_level(simparm::Expert);

    pistonCount.set_user_level(simparm::Expert);
    pistonCount.setHelpID( "#CPUNumber" );
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
    pistonCount.set_user_level(simparm::Beginner);
    pistonCount = 8;
#endif

    if ( localization_replay_mode )
        add_stm_input_modules( *this );
    else
        add_image_input_modules( *this );
    add_output_modules( *this );
    input->publish_meta_info();
}

Config::Config(const Config &c) 
: car_config(c.car_config),
  outputRoot(c.outputRoot->clone()),
  localization_replay_mode(c.localization_replay_mode),
  outputBox(c.outputBox),
  configTarget(c.configTarget),
  auto_terminate(c.auto_terminate),
  pistonCount(c.pistonCount)
{
    if ( c.input.get() )
        create_input( std::auto_ptr<input::Link>(c.input->clone()) );
    input->publish_meta_info();
}

Config::~Config() {
    dStorm::input::InputMutexGuard lock( input::global_mutex() );
    outputRoot.reset( NULL );
    input_listener.reset();
    input.reset();
}

void Config::create_input( std::auto_ptr<input::Link> p ) {
    input_listener = p->notify( boost::bind(&Config::traits_changed, this, _1) );
    input = p;
}

simparm::NodeHandle Config::attach_ui( simparm::NodeHandle at ) {
   current_ui = car_config.attach_ui ( at );
   attach_children_ui( current_ui );
   return current_ui;
}

void Config::attach_children_ui( simparm::NodeHandle at ) {
    input->registerNamedEntries( at );
    if ( ! localization_replay_mode )
        pistonCount.attach_ui(  at  );
    simparm::NodeHandle b = outputBox.attach_ui( at );
    outputRoot->attach_full_ui( b );
    if ( ! localization_replay_mode )
        configTarget.attach_ui( b );
    auto_terminate.attach_ui(  at  );
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
    outputRoot->root_factory().addChoice( o.release() );
}

std::auto_ptr<input::BaseSource> Config::makeSource() const {
    return input->make_source();
}

const input::MetaInfo&
Config::get_meta_info() const {
    return *input->current_meta_info();
}

void Config::traits_changed( boost::shared_ptr<const input::MetaInfo> traits ) {
    if ( ! localization_replay_mode )
        configTarget.set_output_file_basename( traits->suggested_output_basename );
    else
        configTarget.value = "";
    outputRoot->set_output_file_basename( traits->suggested_output_basename );
    if ( traits->provides<output::LocalizedImage>() ) 
        outputRoot->set_trace_capability( *traits->traits<output::LocalizedImage>() );
}

std::auto_ptr< Job > Config::make_job() {
    return std::auto_ptr< Job >( new Car(*this) );
}

}
}
