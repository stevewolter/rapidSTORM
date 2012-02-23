#include "spatial.hpp"
#include <dStorm/image/MetaInfo.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/crop.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace input {
namespace join {

engine::ImageStack merge_data< engine::ImageStack, spatial_tag<2> >::operator()( 
    const input::Traits<engine::ImageStack>& traits,
    const std::vector< input::Source<engine::ImageStack>::iterator >& s,
    spatial_tag<2> ) const
{
    assert( ! s.empty() );
    engine::ImageStack rv( *s[0] );
    for (size_t i = 1; i < s.size(); ++i) {
        std::copy( s[i]->begin(), s[i]->end(),
            std::back_inserter( rv ) );
    }
    return rv;
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
}
}
