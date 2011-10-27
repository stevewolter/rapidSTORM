#include <simparm/BoostUnits.hh>
#include <simparm/BoostOptional.hh>
#include "SampleInfo.h"

#include "debug.h"
#include <simparm/TreeCallback.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/input/Source.h>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter.h>
#include <dStorm/Image_decl.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/input/chain/Context_impl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/DefaultFilterTypes.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/lexical_cast.hpp>

namespace dStorm {
namespace input {
namespace sample_info {

using namespace chain;

class FluorophoreConfig : public simparm::Object {
    simparm::StringEntry description;
    simparm::Entry< boost::units::quantity<boost::units::si::nanolength, double> > emission_wl;

  public:
    FluorophoreConfig(int number);
    void set_traits( FluorophoreTraits& t ) const;
    void registerNamedEntries();
};

class ChainLink;

class Config 
: public simparm::Object {
    simparm::Entry<unsigned long> fluorophore_count;
    boost::ptr_vector< FluorophoreConfig > fluorophores;
    friend class ChainLink;

  public:
    typedef input::chain::DefaultTypes SupportedTypes;

    Config();
    void registerNamedEntries();
    void set_traits( DataSetTraits& ) const;
};

template <typename ForwardedType>
class Input 
: public input::Source<ForwardedType>, public input::Filter
{
    std::auto_ptr< input::Source<ForwardedType> > s;
    Config config;

  public:
    Input(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const Config& config ) 
        : input::Source<ForwardedType>( backend->getNode(), backend->flags ),
          s(backend), config(config) {}

    BaseSource& upstream() { return *s; }

    typedef typename input::Source<ForwardedType>::iterator iterator;
    typedef typename input::Source<ForwardedType>::TraitsPtr TraitsPtr;

    void dispatch(BaseSource::Messages m) { s->dispatch(m); }
    iterator begin() { return s->begin(); }
    iterator end() { return s->end(); }
    TraitsPtr get_traits() {
        DEBUG("Sample info creates traits");
        TraitsPtr rv = s->get_traits();
        config.set_traits(*rv);
        DEBUG("Sample info created traits");
        return rv;
    }
};


class ChainLink 
: public input::chain::Filter, public simparm::TreeListener 
{
    typedef input::chain::DefaultVisitor< Config > Visitor;
    friend class input::chain::DelegateToVisitor;
    friend class Check;

    simparm::Structure<Config> config;
    simparm::Structure<Config>& get_config() { return config; }
    ContextRef context;

    class TraitMaker;

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }
    simparm::Node& getNode() { return config; }

    AtEnd traits_changed( TraitsRef r, Link* l);
    AtEnd context_changed( ContextRef r, Link* l);
    BaseSource* makeSource();
};

}

namespace chain {

template <>
template <typename Type>
bool DefaultVisitor<sample_info::Config>::operator()( Traits<Type>& traits )
{
    config.set_traits( traits );
    return true;
}

template <>
template <typename Type>
bool DefaultVisitor<sample_info::Config>::operator()( std::auto_ptr< Source<Type> > s )
{
    assert( s.get() );
    new_source.reset( new sample_info::Input<Type>(s, config) );
    return true;
}

}

namespace sample_info {

FluorophoreConfig::FluorophoreConfig(int number)
: simparm::Object("Fluorophore" + boost::lexical_cast<std::string>(number), 
                  "Info for fluorophore " + boost::lexical_cast<std::string>(number+1)),
  description("Description", "Description for fluorophore " + boost::lexical_cast<std::string>(number+1)),
  emission_wl("Wavelength", "Emission wavelength", 500.0 * boost::units::si::nanometre)
{
    description.userLevel = 40; /* Not relevant at the moment. */
    emission_wl.userLevel = 40; /* Not relevant at the moment. */
}

void FluorophoreConfig::set_traits( FluorophoreTraits& t ) const
{
    t.description = description();
    t.wavelength = emission_wl() * si::meter / (1E9 * si::nanometre);
}

void FluorophoreConfig::registerNamedEntries() {
    push_back( description );
    push_back( emission_wl );
}

inline void Config::set_traits( DataSetTraits& t ) const
{
    t.fluorophores.clear();
    for ( int i = 0; i < int(fluorophore_count()); ++i)
	fluorophores[i].set_traits( t.fluorophores[i] );
}

Config::Config()
: simparm::Object("SampleInfo", "Sample information"),
  fluorophore_count("FluorophoreCount", "Fluorophore types", 1)
{
    fluorophores.helpID = "FluorophoreTypeCount";
    fluorophores.push_back( new FluorophoreConfig(0) );
    fluorophore_count.min = 1;
    fluorophore_count.increment = 1;
}

void Config::registerNamedEntries() {
    push_back( fluorophore_count );
    for ( int i = 0; i < int(fluorophores.size()); ++i ) {
	fluorophores[i].registerNamedEntries();
	push_back( fluorophores[i] );
    }
}

ChainLink::ChainLink() 
{
    receive_changes_from_subtree( config );
}

ChainLink::ChainLink(const ChainLink& o) 
: Filter(o),
  config(o.config), context(o.context)
{
    receive_changes_from_subtree( config );
}

ChainLink::AtEnd ChainLink::traits_changed( TraitsRef c, Link* l ) { 
    return input::chain::DelegateToVisitor::traits_changed(*this, c, l);
}

chain::Link::AtEnd ChainLink::context_changed( ContextRef c, Link *l )
{
    this->context = c;
    return input::chain::DelegateToVisitor::context_changed(*this, c, l);
}

void ChainLink::operator()(const simparm::Event& e)
{
    if ( e.cause == simparm::Event::ValueChanged) {
        if ( &e.source == &config.fluorophore_count.value ) {
            while ( config.fluorophores.size() < config.fluorophore_count() ) {
                config.fluorophores.push_back( new FluorophoreConfig(config.fluorophores.size()) );
                config.fluorophores.back().registerNamedEntries();
                config.push_back( config.fluorophores.back() );
            }
            for ( boost::ptr_vector< FluorophoreConfig >::iterator i = config.fluorophores.begin(); 
                  i != config.fluorophores.end(); ++i)
            {
                i->viewable = ( (i - config.fluorophores.begin()) < int(config.fluorophore_count()) );
            }
        }
        ost::MutexLock lock( global_mutex() );
        DefaultVisitor<Config> m(config);
        if ( context.get() ) {
            visit_context( m, context );
            notify_of_context_change( context );
        }
        if ( current_traits().get() ) {
            MetaInfo::ConstPtr t( current_traits() );
            visit_traits( m, t );
            notify_of_trait_change( t );
        }
    } else 
	TreeListener::add_new_children(e);
}

BaseSource* ChainLink::makeSource() { 
    DefaultVisitor<Config> visitor(config);
    return specialize_source( visitor, Forwarder::makeSource() );
}

std::auto_ptr<chain::Filter> makeLink() {
    return std::auto_ptr<chain::Filter>( new ChainLink() );
}

}
}
}
