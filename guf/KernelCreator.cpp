#include "KernelCreator.h"
#include "gaussian_psf/BaseExpression.h"
#include "constant_background/model.hpp"

#include "MultiKernelModel.h"

namespace dStorm {
namespace guf {

void KernelCreator::operator()( MultiKernelModel& more, const MultiKernelModel& less, const Spot& a ) const
{
    assert( more.kernel_count() == less.kernel_count() + 1 );

    MultiKernelModel::iterator fresh = more.begin();
    for ( MultiKernelModel::const_iterator old = less.begin(); old != less.end(); ++old, ++fresh )
        fresh->copy( *old );
    more.background_model() = less.background_model();

    fresh->copy( less[0] );
    (*fresh).get_fluorophore_position(0) = a[0];
    (*fresh).get_fluorophore_position(1) = a[1];

    gaussian_psf::Amplitude amp;
    for ( MultiKernelModel::iterator i = more.begin(); i != more.end(); ++i )
        (*fresh).set_amplitude( (*fresh).get_amplitude() * double( (i == fresh) ? fraction : 1 - fraction ));
}

void KernelCreator::operator()( MultiKernelModelStack& more, const MultiKernelModelStack& less, const Spot& a ) const {
    assert( more.size() == less.size() );
    for (size_t i = 0; i < more.size(); ++i)
        (*this)( more[i], less[i], a );
}

}
}
