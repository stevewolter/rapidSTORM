#include "TraitValueFinder.h"

namespace dStorm {
namespace guf {

TraitValueFinder::TraitValueFinder( 
    const dStorm::engine::JobInfo& info, 
    const dStorm::traits::Optics& plane )
: info(info), plane(plane), 
    psf( plane.psf_size(info.fluorophore) ),
    is_3d( boost::get<traits::Polynomial3D>(info.traits.depth_info.get_ptr()) ) 
{
    assert( psf.is_initialized() );
}

}
}
