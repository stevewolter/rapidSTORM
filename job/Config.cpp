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
#include "base/Engine.h"
#include "input/Forwarder.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "output/Basename.h"
#include "output/FilterSource.h"
#include "output/SourceFactory.h"
#include "simparm/Menu.h"
#include "simparm/text_stream/RootNode.h"
#include "simparm/TreeRoot.h"
#include "simparm/TreeEntry.h"
#include "ui/serialization/Node.h"

#include "job/Car.h"
#include "job/Config.h"
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
  thread_count_("CPUNumber", "Number of CPUs to use", 1),
  configTarget("SaveConfigFile", "Job options file", "-settings.txt"),
  auto_terminate("AutoTerminate", "Automatically terminate finished jobs", false)
{
    configTarget.set_user_level(simparm::Beginner);
    auto_terminate.set_user_level(simparm::Expert);

    thread_count_.set_user_level(simparm::Expert);
    thread_count_.setHelpID( "#CPUNumber" );
    thread_count_.setHelp("Use this many parallel threads to compute the "
                          "STORM result. If you notice a low CPU usage, "
                          "raise this value to the number of cores you "
                          "have.");
#if defined(_SC_NPROCESSORS_ONLN)
    int pn = sysconf(_SC_NPROCESSORS_ONLN);
    thread_count_ = (pn == 0) ? 8 : pn;
#elif defined(HAVE_WINDOWS_H)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    thread_count_ = info.dwNumberOfProcessors;
#else
    thread_count_.set_user_level(simparm::Beginner);
    thread_count_ = 8;
#endif

    if ( localization_replay_mode )
        input = create_localizations_input();
    else
        input = create_image_input();
    add_output_modules( *this );
    input->publish_meta_info();
}

Config::Config(const Config &c) 
: car_config(c.car_config),
  outputRoot(c.outputRoot->clone()),
  localization_replay_mode(c.localization_replay_mode),
  outputBox(c.outputBox),
  thread_count_(c.thread_count_),
  configTarget(c.configTarget),
  auto_terminate(c.auto_terminate)
{
    if ( c.input.get() )
        set_input( std::unique_ptr<input::Link<output::LocalizedImage>>(c.input->clone()) );
    input->publish_meta_info();
}

Config::~Config() {
    dStorm::input::InputMutexGuard lock( input::global_mutex() );
    outputRoot.reset( NULL );
    input_listener.reset();
    input.reset();
}

void Config::set_input( std::unique_ptr<input::Link<output::LocalizedImage>> p ) {
    input_listener = p->notify( boost::bind(&Config::traits_changed, this, _1) );
    input = std::move(p);
}

simparm::NodeHandle Config::attach_ui( simparm::NodeHandle at ) {
   current_ui = car_config.attach_ui ( at );
   attach_children_ui( current_ui );
   return current_ui;
}

void Config::attach_children_ui( simparm::NodeHandle at ) {
    input->registerNamedEntries( at );
    if ( ! localization_replay_mode )
        thread_count_.attach_ui(  at  );
    simparm::NodeHandle b = outputBox.attach_ui( at );
    outputRoot->attach_full_ui( b );
    if ( ! localization_replay_mode )
        configTarget.attach_ui( b );
    auto_terminate.attach_ui(  at  );
}

void Config::add_output( std::auto_ptr<output::OutputSource> o ) {
    outputRoot->root_factory().addChoice( o.release() );
}

std::unique_ptr<input::BaseSource> Config::makeSource() const {
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
}

std::auto_ptr< Job > Config::make_job() {
    return std::auto_ptr< Job >( new Car(*this) );
}

}
}
