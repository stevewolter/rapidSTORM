#include <simparm/BoostUnits.hh>
#include <simparm/BoostOptional.hh>
#include "SampleInfo.h"

#include "debug.h"
#include <simparm/TreeCallback.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/localization/Traits.h>
#include <simparm/Structure.hh>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/Image_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/ImageTraits_impl.h>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace input {
namespace sample_info {

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
    Config();
    void registerNamedEntries();
    void set_traits( DataSetTraits& ) const;
};

template <typename ForwardedType>
class Input 
: public input::AdapterSource<ForwardedType>
{
    Config config;
    void modify_traits( input::Traits<ForwardedType>& t ) { config.set_traits(t); }

  public:
    Input(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const Config& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) {}
};


class ChainLink 
: public input::Method<ChainLink>, public simparm::TreeListener 
{
    friend class input::Method<ChainLink>;
    friend class Check;

    simparm::Structure<Config> config;
    simparm::Structure<Config>& get_config() { return config; }

    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& t ) {
        config.set_traits( t );
    }
    template <typename Type>
    Input<Type>* make_source( std::auto_ptr< input::Source<Type> > s ) {
        return new Input<Type>(s, config);
    }

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    simparm::Node& getNode() { return config; }
};

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
    fluorophore_count.helpID = "FluorophoreTypeCount";
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
: input::Method<ChainLink>(o), config(o.config)
{
    receive_changes_from_subtree( config );
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
        input::InputMutexGuard lock( global_mutex() );
        republish_traits();
    } else 
	TreeListener::add_new_children(e);
}

std::auto_ptr<Link> makeLink() {
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
}
