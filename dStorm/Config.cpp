#define DSTORM_CARCONFIG_CPP
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "debug.h"
#include "Config.h"
#include "engine/SpotFitterFactory.h"
#include "engine/SpotFinder.h"
#include "input/Config.h"
#include "output/FilterSource.h"
#include "output/SourceFactory.h"
#include "output/Basename.h"
#include "engine/ClassicEngine.h"
#include "Engine.h"

#include "helpers/thread.h"
#include <sstream>

#include <simparm/ChoiceEntry_Impl.hh>
#include "input/chain/MetaInfo.h"
#include "input/chain/Context.h"
#include "input/chain/Alternatives.h"
#include "input/InputMutex.h"

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

using namespace std;

namespace boost {
    template <>
    inline dStorm::input::chain::Filter* 
    new_clone<dStorm::input::chain::Filter>
        ( const dStorm::input::chain::Filter& o )
        { return o.clone(); }
    template <>
    inline dStorm::engine::spot_finder::Factory* 
    new_clone<dStorm::engine::spot_finder::Factory>
        ( const dStorm::engine::spot_finder::Factory& o )
        { return o.clone(); }
    template <>
    inline dStorm::engine::spot_fitter::Factory* 
    new_clone<dStorm::engine::spot_fitter::Factory>
        ( const dStorm::engine::spot_fitter::Factory& o )
        { return o.clone(); }
}

namespace dStorm {

class Config::TreeRoot : public simparm::Object, public output::FilterSource
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

class Config::EngineChoice
: input::chain::Link {
    input::chain::Alternatives alternatives;
    boost::ptr_list<input::chain::Filter> engines;
    boost::ptr_list<engine::spot_finder::Factory> finders;
    boost::ptr_list<engine::spot_fitter::Factory> fitters;

    std::list<engine::ClassicEngine*> classic_engines;

    input::chain::Context::Ptr initial_context;

    TreeRoot* outputRoot;

    template <typename Type>
    void clone( const boost::ptr_list<Type>& l ) {
        typedef typename boost::ptr_list<Type>::const_iterator const_iterator;
        for ( const_iterator i = l.begin(); i != l.end(); ++i )
            add( std::auto_ptr<Type>( i->clone() ) );
    }

    AtEnd traits_changed( TraitsRef traits, input::chain::Link* el ) {
        DEBUG("Traits for input chain base changed to " << traits.get());
        assert( el == &alternatives );
        if ( traits.get() == NULL ) return AtEnd();

        DEBUG("Basename declared in traits is " << traits->suggested_output_basename );
        output::Basename bn( traits->suggested_output_basename );
        bn.set_variable("run", "snapshot");
        
        DEBUG("Got new basename " << bn << " for config " << this);
        outputRoot->set_output_file_basename( bn );
        if ( traits->provides<output::LocalizedImage>() ) 
        {
            outputRoot->set_trace_capability( *traits->traits<output::LocalizedImage>() );
        }

        std::string filename = bn.new_basename();
        if ( filename != initial_context->output_basename ) {
            initial_context->output_basename = filename;
            return alternatives.context_changed( initial_context, this );
        } else
            return AtEnd();
    }

    AtEnd context_changed( ContextRef, Link* ) {
        assert(false); 
        throw std::logic_error("dStorm::input::Config::ChainTerminus can have no context"); 
    }

    void set_initial_context( bool show_error ) {
        DEBUG("Setting initial context " << show_error << " and " << initial_context->throw_errors);
        if ( initial_context->throw_errors == show_error )
            return;
        if ( ! initial_context.unique() )
            initial_context.reset( initial_context->clone() );
        initial_context->throw_errors = show_error;
        ost::MutexLock lock( input::global_mutex() );
        alternatives.context_changed( initial_context, this );
    }
    
  public:
    EngineChoice(Config& config) 
        : alternatives("Engine", "Choose engine") ,
          initial_context( new input::chain::Context() )
    {
        assert( config.outputRoot.get() );
        outputRoot = config.outputRoot.get();

        ost::MutexLock lock( input::global_mutex() );
        alternatives.set_more_specialized_link_element( 
            &config._inputConfig->get_link_element() );

        Link::set_upstream_element( alternatives, *this, Add );
        initial_context->need_multiple_concurrent_iterators = true;
        alternatives.context_changed( initial_context, this );
    }
    EngineChoice(const EngineChoice& o, Config& config) 
    : alternatives(o.alternatives),
      finders(o.finders),
      fitters(o.fitters),
      initial_context(o.initial_context)
    {
        assert( config.outputRoot.get() );
        outputRoot = config.outputRoot.get();

        ost::MutexLock lock( input::global_mutex() );
        alternatives.set_more_specialized_link_element( 
            &config._inputConfig->get_link_element() );

        Link::set_upstream_element( alternatives, *this, Add );
        clone( o.engines );

        if ( o.alternatives.isValid() )
            alternatives.choose( o.alternatives.value().getNode().getName() );
    }
    ~EngineChoice() {
        engines.clear();
        Link::set_upstream_element( alternatives, *this, Remove );
    }

    input::BaseSource* makeSource() { 
        ost::MutexLock lock( input::global_mutex() );
        return alternatives.makeSource(); 
    }
    Link* clone() const { return new EngineChoice(*this); }

    simparm::Node& getNode() { return alternatives.getNode(); }

    void add( std::auto_ptr<input::chain::Filter> e ) {
        ost::MutexLock lock( input::global_mutex() );
        engine::ClassicEngine* ce = dynamic_cast<engine::ClassicEngine*>(e.get());
        if ( ce != NULL ) classic_engines.push_back(ce);
        alternatives.add_choice( *e, alternatives.endChoices() );
        DEBUG("Adding engine " << e->getNode().getName() << " to " << this );
        engines.push_back(e);
    }

    void add( std::auto_ptr<engine::spot_finder::Factory> s ) {
        ost::MutexLock lock( input::global_mutex() );
        for ( std::list<engine::ClassicEngine*>::iterator 
              i = classic_engines.begin(); i != classic_engines.end(); ++i )
            (*i)->add_spot_finder( *s );
        finders.push_back(s);
    }

    void add( std::auto_ptr<engine::spot_fitter::Factory> s ) {
        ost::MutexLock lock( input::global_mutex() );
        for ( std::list<engine::ClassicEngine*>::iterator 
              i = classic_engines.begin(); i != classic_engines.end(); ++i )
            (*i)->add_spot_fitter( *s );
        fitters.push_back(s);
    }

    const input::chain::MetaInfo& get_meta_info() {
        if ( alternatives.current_traits().get() ) {
            return *alternatives.current_traits();
        } else {
            DEBUG("Traits are invalid, trying to enforce exception throwing");
            set_initial_context(true);
            if ( alternatives.current_traits().get() ) {
                return *alternatives.current_traits();
            } else {
                DEBUG("No traits set for current engine");
                DEBUG("Current engine is " << ((alternatives.isValid()) ? alternatives.value().getNode().getName() : "invalid"));
                throw std::runtime_error("No usable configuration was found");
            }
        }
    }

    void insert_new_node( std::auto_ptr<Link> link ) {
        alternatives.insert_new_node(link);
    }
};

Config::TreeRoot::TreeRoot()
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

Config::Config() 
: Set("Car", "Job options"),
  _inputConfig( new input::Config() ),
  outputRoot( new TreeRoot() ),
  engine_choice( new EngineChoice(*this) ),
  inputConfig(*_inputConfig),
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

   helpMenu.push_back( std::auto_ptr<simparm::Node>(
        new simparm::URI("BugTracker", "Report a bug", 
            "https://idefix.biozentrum.uni-wuerzburg.de/bugzilla3" ) ),
            true );

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

    registerNamedEntries();
}

Config::Config(const Config &c) 
: simparm::Set(c),
  _inputConfig(c.inputConfig.clone()),
  outputRoot(c.outputRoot->clone()),
  engine_choice( new EngineChoice(*c.engine_choice, *this) ),
  inputConfig(*_inputConfig),
  outputSource(*outputRoot),
  outputConfig(outputRoot->root_factory()),
  helpMenu( c.helpMenu ),
  outputBox(c.outputBox),
  configTarget(c.configTarget),
  auto_terminate(c.auto_terminate),
  pistonCount(c.pistonCount)
{
    registerNamedEntries();
    DEBUG("Copied Car config");
}

Config::~Config() {
    ost::MutexLock lock( input::global_mutex() );
    outputRoot.reset( NULL );
    engine_choice.reset( NULL );
    _inputConfig.reset( NULL );
}

void Config::registerNamedEntries() {
   DEBUG("Registering named entries of CarConfig with " << size() << " elements before registering");
   outputBox.push_back( *outputRoot );
   push_back( inputConfig );
   push_back( engine_choice->getNode() );
   push_back( pistonCount );
   push_back( outputBox );
   push_back( configTarget );
   push_back( auto_terminate );
   push_back( helpMenu );
   DEBUG("Registered named entries of CarConfig with " << size() << " elements after registering");
}

void Config::add_engine( std::auto_ptr<input::chain::Filter> engine) {
    engine_choice->add( engine );
}

void Config::add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> engine) {
    engine_choice->add( engine );
}

void Config::add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> engine) {
    engine_choice->add( engine );
}

std::auto_ptr<input::BaseSource> Config::makeSource() {
    return std::auto_ptr<input::BaseSource>( engine_choice->makeSource() );
}

const input::chain::MetaInfo&
Config::get_meta_info() const {
    return engine_choice->get_meta_info();
}

}
