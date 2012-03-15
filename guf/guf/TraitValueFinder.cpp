#include "TraitValueFinder.h"

namespace dStorm {
namespace guf {

TraitValueFinder::TraitValueFinder( 
    const int fluorophore, 
    const dStorm::traits::Optics& plane )
: fluorophore(fluorophore), plane(plane), 
    psf( plane.psf_size(fluorophore) ),
    is_3d( boost::get<traits::Polynomial3D>(plane.depth_info().get_ptr()) ) 
{
    assert( psf.is_initialized() );
}

}
}
