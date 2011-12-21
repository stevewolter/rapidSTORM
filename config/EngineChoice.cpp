#ifndef DSTORM_CONFIG_ENGINE_CHOICE_H
#define DSTORM_CONFIG_ENGINE_CHOICE_H

#include "EngineChoice.h"
#include "Grand.h"
#include <dStorm/input/chain/Alternatives.h>
#include <dStorm/engine/ClassicEngine.h>
#include <dStorm/input/InputMutex.h>

namespace boost {
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

class EngineChoice
: public IEngineChoice {
    input::chain::Alternatives alternatives;
    boost::ptr_list<engine::spot_finder::Factory> finders;
    boost::ptr_list<engine::spot_fitter::Factory> fitters;

    std::list<engine::ClassicEngine*> classic_engines;

    GrandConfig& config;

    template <typename Type>
    void clone( const boost::ptr_list<Type>& l ) {
        typedef typename boost::ptr_list<Type>::const_iterator const_iterator;
        for ( const_iterator i = l.begin(); i != l.end(); ++i )
            add( std::auto_ptr<Type>( i->clone() ) );
    }

    AtEnd traits_changed( TraitsRef traits, input::chain::Link* el ) {
        assert( el == &alternatives );
        if ( traits.get() != NULL ) 
            config.traits_changed(*traits);
        return AtEnd();
    }

  public:
    EngineChoice(GrandConfig& config) 
        : alternatives("Engine", "Choose engine", true),
          config(config)
    {
        Link::set_upstream_element( alternatives, *this, Add );
    }
    EngineChoice(const EngineChoice& o, GrandConfig& config) 
    : alternatives(o.alternatives),
      finders(o.finders),
      fitters(o.fitters),
      config(config)
    {
        traits_changed( alternatives.current_traits(), &alternatives );
        Link::set_upstream_element( alternatives, *this, Add );
    }
    ~EngineChoice() { }

    input::BaseSource* makeSource() { 
        ost::MutexLock lock( input::global_mutex() );
        return alternatives.makeSource(); 
    }
    Link* clone() const { return new EngineChoice(*this); }

    void registerNamedEntries( simparm::Node& n ) 
        { alternatives.registerNamedEntries(n); }
    std::string name() const { return alternatives.name(); }
    std::string description() const { return alternatives.description(); }

    void add( std::auto_ptr<input::chain::Link> e ) {
        ost::MutexLock lock( input::global_mutex() );
        engine::ClassicEngine* ce = dynamic_cast<engine::ClassicEngine*>(e.get());
        if ( ce != NULL ) classic_engines.push_back(ce);
        alternatives.add_choice( e );
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

    void insert_new_node( std::auto_ptr<Link> link, Place p ) {
        if ( p == AsEngine )
            add( link );
        else
            alternatives.insert_new_node(link,p);
    }
};

std::auto_ptr< IEngineChoice > make_engine_choice(GrandConfig& c)
    { return std::auto_ptr< IEngineChoice >( new EngineChoice(c) ); }
std::auto_ptr< IEngineChoice > copy_engine_choice(const IEngineChoice& f, GrandConfig& c)
    { return std::auto_ptr< IEngineChoice >( new EngineChoice(dynamic_cast<const EngineChoice&>(f), c) ); }


}

#endif
