#include "debug.h"
#include "Buffer_impl.h"
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>
#include <dStorm/input/Method.hpp>

namespace dStorm { 
namespace input { 

class BufferChainLink 
: public input::Method<BufferChainLink>
{
    simparm::Object config;

    friend class input::Method<BufferChainLink>;
    template <typename Type>
    Source<Type>* make_source( std::auto_ptr< Source<Type> > p ) 
        { return new Buffer<Type,false>(p); }
    template <typename Type>
    void update_traits( chain::MetaInfo&, Traits<Type>& ) {}
    template <typename Type>
    bool changes_traits( const chain::MetaInfo&, const Traits<Type>& )
        { return false; }

  public:
    BufferChainLink();
    simparm::Node& getNode() { return config; }
};

BufferChainLink::BufferChainLink() 
: config("Buffer", "Buffer") 
{
}

std::auto_ptr<chain::Link> makeBufferChainLink() { 
    return std::auto_ptr<chain::Link>( new BufferChainLink() );
}

}
}
