#include "guf/TraitValueFinder.h"

namespace dStorm {
namespace guf {

TraitValueFinder::TraitValueFinder( 
    const int fluorophore, 
    const dStorm::traits::Optics& plane )
: fluorophore(fluorophore), plane(plane)
{
}

}
}
