#include "TraitValueFinder.h"

namespace dStorm {
namespace guf {

TraitValueFinder::TraitValueFinder( 
    const int fluorophore, 
    const dStorm::traits::Optics& plane )
: fluorophore(fluorophore), plane(plane), 
    psf( plane.psf_size(fluorophore) )
{
    assert( psf.is_initialized() );
}

}
}
