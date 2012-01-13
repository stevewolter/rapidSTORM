#include "KernelCreator.h"
#include "guf/psf/BaseExpression.h"
#include "guf/constant_background.hpp"

#include "guf/psf/ostream.h"

namespace dStorm {
namespace guf {

void KernelCreator::operator()( FittedPlane& more, const FittedPlane& less, const Spot& a ) const
{
    assert( more.kernel_count() == less.kernel_count() + 1 );

    FittedPlane::iterator fresh = more.begin();
    for ( FittedPlane::const_iterator old = less.begin(); old != less.end(); ++old, ++fresh )
        fresh->copy( *old );
    more.background_model() = less.background_model();

    fresh->copy( less[0] );
    (*fresh)( PSF::Mean<0>() ) = a[0];
    (*fresh)( PSF::Mean<1>() ) = a[1];

    PSF::Amplitude amp;
    for ( FittedPlane::iterator i = more.begin(); i != more.end(); ++i )
        (*fresh)( amp ) = *(*fresh)( amp ) * double( (i == fresh) ? fraction : 1 - fraction );
}

void KernelCreator::operator()( FitPosition& more, const FitPosition& less, const Spot& a ) const {
    assert( more.size() == less.size() );
    for (size_t i = 0; i < more.size(); ++i)
        (*this)( more[i], less[i], a );
}

}
}
