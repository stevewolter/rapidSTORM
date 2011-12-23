#include "spatial.hpp"
#include <dStorm/ImageTraits.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/crop.h>
#include <dStorm/image/iterator.h>

namespace dStorm {
namespace input {
namespace join {

template <int Dim>
engine::Image merge_data< engine::Image, spatial_tag<Dim> >::operator()( 
    const input::Traits<engine::Image>& traits,
    const std::vector< input::Source<engine::Image>::iterator >& s,
    spatial_tag<Dim> ) const
{
    engine::Image rv( traits.size, s[0]->frame_number() );
    engine::Image::Size offset = engine::Image::Size::Constant(0 * boost::units::camera::pixel);
    for (size_t i = 0; i < s.size(); ++i) {
        engine::Image::Size window = s[i]->sizes();
        for (int r = 0; r < window.rows(); ++r ) 
            window[r] -= 1 * boost::units::camera::pixel ;
        std::copy( s[i]->begin(), s[i]->end(), 
            crop( rv, offset, offset + window ).begin() );
        offset[Dim] += s[i]->sizes()[Dim];
    }
    return rv;
}

template <int Dim>
typename traits_merger<engine::Image>::result_type
merge_traits< engine::Image, spatial_tag<Dim> >::
    operator()( const typename traits_merger<engine::Image>::argument_type& images ) const
{
    std::auto_ptr< Traits<engine::Image> > rv( new Traits<engine::Image>(*images[0]) );
    for (size_t i = 1; i < images.size(); ++i) {
        for (int d = 0; d < rv->size.rows(); ++d) {
            assert( images[i] );
            assert( images[i]->size.rows() >= rv->size.rows() );
            if ( d == Dim )
                rv->size[d] += images[i]->size[d];
            else if ( rv->size[d] != images[i]->size[d] ) {
                std::stringstream ss;
                ss << "Image dimension " << d << " does not agree between channels 0 and " << i 
                    << ", cannot join images" << std::endl;
                throw std::runtime_error(ss.str());
            }
        }
        if ( Dim == 2 ) {
            std::copy( images[i]->planes.begin(), images[i]->planes.end(),
                back_inserter( rv->planes ) );
        }
    }
    return rv;
}

template class merge_traits< engine::Image, spatial_tag<0> >;
template class merge_traits< engine::Image, spatial_tag<1> >;
template class merge_traits< engine::Image, spatial_tag<2> >;
template class merge_data< engine::Image, spatial_tag<0> >;
template class merge_data< engine::Image, spatial_tag<1> >;
template class merge_data< engine::Image, spatial_tag<2> >;

}
}
}
