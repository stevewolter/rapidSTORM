#include "spatial.hpp"
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
        traits.reset(  merge_traits<Type,Tag>()(base_traits).release()  );
        return traits;
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

  private:
    typedef std::vector< std::shared_ptr<Base> > Sources;

    Sources sources;
    std::vector< std::unique_ptr< const input::Traits<engine::ImageStack> > > base_traits;
    Base::TraitsPtr traits;
    std::vector<std::unique_ptr<simparm::Object>> connection_nodes;

    void attach_ui_( simparm::NodeHandle n ); 
};

void Source::attach_ui_( simparm::NodeHandle n ) {
    for (size_t i = 0; i < sources.size(); ++i) {
        std::auto_ptr< simparm::Object > object( 
            new simparm::Object("Channel" + boost::lexical_cast<std::string>(i), "") );
        sources[i]->attach_ui( object->attach_ui( n ) ); 
        connection_nodes.push_back( object );
    }
}

bool merge_data< engine::ImageStack, spatial_tag<2> >::operator()( 
    const input::Traits<engine::ImageStack>& traits,
    const std::vector< input::Source<engine::ImageStack>::iterator >& s,
    spatial_tag<2>,
    engine::ImageStack* target) const
{
    assert( ! s.empty() );
    if (!s[0]->GetNext(thread, target)) {
        return false;
    }

    for (size_t i = 1; i < s.size(); ++i) {
        engine::ImageStack next_plane;
        if (!s[i]->GetNext(thread, &next_plane)) {
            return false;
        }
        target->push_back(next_plane);
    }

    return true;
}

std::auto_ptr< Traits<engine::ImageStack> >
merge_traits< engine::ImageStack, spatial_tag<2> >::operator()
    ( const argument_type& images ) const
{
    std::auto_ptr< Traits<engine::ImageStack> > rv( new Traits<engine::ImageStack>(*images[0]) );
    for (size_t i = 1; i < images.size(); ++i) {
        std::copy( images[i]->begin(), images[i]->end(),
            std::back_inserter( *rv ) );
    }
    return rv;
}

}

std::unique_ptr<input::Source<engine::ImageStack>> Create(
        std::vector<std::unique_ptr<input::Source<engine::ImageStack>>> sources) {
    return std::make_unique<Source>(sources);
}

}
}
