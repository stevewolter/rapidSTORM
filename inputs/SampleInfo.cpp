#include "simparm/BoostUnits.h"
#include "simparm/BoostOptional.h"
#include "inputs/SampleInfo.h"

#include <boost/lexical_cast.hpp>
#include <boost/ptr_container/ptr_vector.hpp>

#include "debug.h"
#include "engine/Image.h"
#include "engine/InputTraits.h"
#include "helpers/make_unique.hpp"
#include "input/AdapterSource.h"
#include "input/FilterFactory.h"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "Localization.h"
#include "localization/Traits.h"
#include "localization/Traits.h"
#include "simparm/dummy_ui/fwd.h"
#include "simparm/FileEntry.h"
#include "UnitEntries/PixelSize.h"
#include "units/nanolength.h"

namespace dStorm {
namespace input {
namespace sample_info {

class Config 
{
    simparm::Object name_object;
  public:
    simparm::Entry<unsigned long> fluorophore_count;

    Config();
    void attach_ui( simparm::NodeHandle );
};

class Input 
: public input::AdapterSource<engine::ImageStack>
{
    int fluorophore_count;
    void modify_traits( input::Traits<engine::ImageStack>& t ) {
        t.fluorophore_count = fluorophore_count;
    }
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Input(
        std::unique_ptr< input::Source<engine::ImageStack> > backend,
        int fluorophore_count ) 
        : input::AdapterSource<engine::ImageStack>( std::move(backend) ), fluorophore_count(fluorophore_count) {}
};


class SourceFactory 
: public input::FilterFactory<engine::ImageStack>
{
    Config config;

    SourceFactory* clone() const { return new SourceFactory(*this); }
    boost::shared_ptr<const input::Traits<engine::ImageStack>>
    make_meta_info( input::MetaInfo&, boost::shared_ptr<const input::Traits<engine::ImageStack>> t ) {
        if (!t) return t;

        boost::shared_ptr<input::Traits<engine::ImageStack>> n(
            new input::Traits<engine::ImageStack>(*t));
        config.fluorophore_count.set_visibility( t->plane_count() > 1 );
        n->fluorophore_count = config.fluorophore_count();
        return n;
    }
    std::unique_ptr<input::Source<engine::ImageStack>>
    make_source( std::unique_ptr< input::Source<engine::ImageStack> > s ) {
        return std::unique_ptr<input::Source<engine::ImageStack>>(
            new Input(std::move(s), config.fluorophore_count()));
    }
    simparm::BaseAttribute::ConnectionStore listening;

  public:
    void attach_ui( simparm::NodeHandle at, std::function<void()> cb ) OVERRIDE { 
        listening = config.fluorophore_count.value.notify_on_value_change( cb );
        config.attach_ui( at ); 
    }
};

Config::Config()
: name_object( "SampleInfo", "Sample information"),
  fluorophore_count("FluorophoreCount", 1)
{
    fluorophore_count.min = 1;
    fluorophore_count.increment = 1;
    fluorophore_count.hide();
}

void Config::attach_ui( simparm::NodeHandle at ) {
    simparm::NodeHandle r = name_object.attach_ui( at );
    fluorophore_count.attach_ui( r );
}

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create() {
    return make_unique<SourceFactory>();
}

}
}
}
