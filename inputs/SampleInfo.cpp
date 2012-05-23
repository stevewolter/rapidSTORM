#include <simparm/BoostUnits.hh>
#include <simparm/BoostOptional.hh>
#include "SampleInfo.h"

#include "debug.h"
#include <simparm/FileEntry.hh>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/UnitEntries/PixelSize.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/NodeHandle.hh>

namespace dStorm {
namespace input {
namespace sample_info {

class ChainLink;

class Config 
{
    simparm::Object name_object;
    simparm::Entry<unsigned long> fluorophore_count;
    friend class ChainLink;

  public:
    Config();
    void attach_ui( simparm::Node& );
    void set_traits( DataSetTraits& ) const;
};

template <typename ForwardedType>
class Input 
: public input::AdapterSource<ForwardedType>
{
    Config config;
    void modify_traits( input::Traits<ForwardedType>& t ) { config.set_traits(t); }
    void attach_local_ui_( simparm::Node& ) {}

  public:
    Input(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const Config& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) {}
};


class ChainLink 
: public input::Method<ChainLink>
{
    friend class input::Method<ChainLink>;
    friend class Check;

    Config config;

    void notice_traits( const input::MetaInfo&, const input::Traits<engine::ImageStack>& t ) {
        config.fluorophore_count.viewable = ( t.plane_count() > 1 );
    }
    template <typename Type>
    void notice_traits( const input::MetaInfo&, const input::Traits<Type>& t ) {
        config.fluorophore_count.viewable = false;
    }
    template <typename Type>
    void update_traits( input::MetaInfo&, input::Traits<Type>& t ) {
        config.set_traits( t );
    }
    template <typename Type>
    Input<Type>* make_source( std::auto_ptr< input::Source<Type> > s ) {
        return new Input<Type>(s, config);
    }
    simparm::BaseAttribute::ConnectionStore listening;

  public:
    static std::string getName() { return "SampleInfo"; }
    void attach_ui( simparm::Node& at ) { 
        listening = config.fluorophore_count.value.notify_on_value_change( 
            boost::bind( &input::Method<ChainLink>::republish_traits_locked, this ) );
        config.attach_ui( at ); 
    }
};

inline void Config::set_traits( DataSetTraits& t ) const
{
    t.fluorophores.clear();
    for ( int i = 0; i < int(fluorophore_count()); ++i)
	t.fluorophores[i].ident = i;
}

Config::Config()
: name_object( ChainLink::getName(), "Sample information"),
  fluorophore_count("FluorophoreCount", "Fluorophore types", 1)
{
    fluorophore_count.helpID = "FluorophoreTypeCount";
    fluorophore_count.min = 1;
    fluorophore_count.increment = 1;
    fluorophore_count.viewable = false;
}

void Config::attach_ui( simparm::Node& at ) {
    simparm::NodeRef r = name_object.attach_ui( at );
    fluorophore_count.attach_ui( r );
}

std::auto_ptr<Link> makeLink() {
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
}
