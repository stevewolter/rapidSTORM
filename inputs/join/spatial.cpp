#include "inputs/join/spatial.h"

#include "debug.h"
#include <boost/lexical_cast.hpp>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/crop.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace spatial_join {
namespace {

class Source : public input::Source<engine::ImageStack> {
  public:
    typedef input::Source<engine::ImageStack> Base;

    Source( std::vector<std::unique_ptr<Base>> sources ) : sources(std::move(sources)) {}

    Base::TraitsPtr get_traits( Wishes r ) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i) {
            base_traits.push_back( (*i)->get_traits(r) );
        }
        return Base::TraitsPtr(merge_traits(base_traits, tag()).release());
    }

    void dispatch(BaseSource::Messages m) {
        for (typename Sources::iterator i = sources.begin(); i != sources.end(); ++i)
            (*i)->dispatch(m);
    }

    Capabilities capabilities() const {
        Capabilities rv;
        rv.set();
        for (const auto& source : sources) {
            rv = rv.to_ulong() & source->capabilities().to_ulong();
        }
        return rv;
    }

    bool GetNext(int thread, engine::ImageStack* target) OVERRIDE;
    void set_thread_count(int num_threads) {
        for (const auto& source : sources) {
            source->set_thread_count(num_threads);
        }
    }

  private:
    typedef std::vector< std::unique_ptr<Base> > Sources;

    Sources sources;
    std::vector< Base::ConstTraitsPtr > base_traits;
    std::vector<std::unique_ptr<simparm::Object>> connection_nodes;

    void attach_ui_( simparm::NodeHandle n ); 
};

void Source::attach_ui_( simparm::NodeHandle n ) {
    for (size_t i = 0; i < sources.size(); ++i) {
        std::unique_ptr< simparm::Object > object( 
            new simparm::Object("Channel" + boost::lexical_cast<std::string>(i), "") );
        sources[i]->attach_ui( object->attach_ui( n ) ); 
        connection_nodes.push_back( std::move(object) );
    }
}

bool Source::GetNext(int thread, engine::ImageStack* target) {
    assert( ! sources.empty() );
    if (!sources[0]->GetNext(thread, target)) {
        return false;
    }
    DEBUG("Joining inputs for image " << target->frame_number().value());

    for (size_t i = 1; i < sources.size(); ++i) {
        engine::ImageStack next_plane;
        if (!sources[i]->GetNext(thread, &next_plane)) {
            return false;
        }
        for (int plane = 0; plane < next_plane.plane_count(); ++plane) {
            target->push_back(next_plane.plane(plane));
        }
    }

    return true;
}

}

std::unique_ptr< input::Traits<engine::ImageStack> > merge_traits(
        const std::vector< boost::shared_ptr< const input::Traits<engine::ImageStack> > >& base_traits,
        tag t) {
    typedef input::Traits<engine::ImageStack> Traits;
    std::unique_ptr< Traits > traits( new Traits(*base_traits[0]) );
    for (size_t i = 1; i < base_traits.size(); ++i) {
        std::copy( base_traits[i]->begin(), base_traits[i]->end(),
            std::back_inserter( *traits ) );
    }
    return std::move(traits);
}

std::unique_ptr<input::Source<engine::ImageStack>> Create(
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> sources) {
    return std::unique_ptr<input::Source<engine::ImageStack>>(new Source(std::move(sources)));
}

}
}
