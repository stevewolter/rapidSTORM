#include "debug.h"
#include "NaiveFitter.h"
#include "ModelledFitter.h"
#include "guf/MultiKernelLambda.h"
#include "gaussian_psf/free_form.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/expressions.h"
#include "measured_psf/expressions.h"
#include "measured_psf/fixed_form.h"
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/traits/optics.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/InputTraits.h>
#include <nonlinfit/Bind.h>
#include <dStorm/threed_info/No3D.h>

namespace dStorm {
namespace guf {

template <int Kernels, typename Assignment, typename Lambda>
inline NaiveFitter::Ptr
create2( const Config& c, const dStorm::engine::JobInfo& i )
{
    typedef typename MultiKernelLambda<
        nonlinfit::Bind< Lambda ,Assignment> ,Kernels>
        ::type F;
    return std::auto_ptr<NaiveFitter>( new ModelledFitter<F>(c,i) );
}

template <>
inline NaiveFitter::Ptr
create2<2,gaussian_psf::FreeForm,gaussian_psf::No3D>( const Config& c, const dStorm::engine::JobInfo& i )
{
    throw std::runtime_error("Two kernels and free-sigma fitting can't be combined, sorry");
}


template <int Kernels>
NaiveFitter::Ptr
NaiveFitter::create(
    const Config& c,
    const dStorm::engine::JobInfo& info )
{
    bool consistently_no_3d = true;
    for ( input::Traits< engine::ImageStack >::const_iterator i = info.traits.begin(); i != info.traits.end(); ++i )
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir) {
            bool is_no3d = dynamic_cast< const threed_info::No3D* >( i->optics.depth_info(dir).get() );
            consistently_no_3d = consistently_no_3d && is_no3d;
        }

    if ( c.free_sigmas() && ! consistently_no_3d )
        throw std::runtime_error("Free-sigma fitting is limited to 2D");
    else if ( true ) {
        return create2<Kernels,measured_psf::FreeForm,measured_psf::Model>(c,info);
    } else if ( c.free_sigmas() )
        return create2<Kernels,gaussian_psf::FreeForm,gaussian_psf::No3D>(c,info);
    else if ( consistently_no_3d )
        return create2<Kernels,gaussian_psf::FixedForm,gaussian_psf::No3D>(c,info);
    else
        return create2<Kernels,gaussian_psf::FixedForm,gaussian_psf::Spline3D>( c, info );
}

template NaiveFitter::Ptr NaiveFitter::create<1>( const Config& c, const dStorm::engine::JobInfo& i );
template NaiveFitter::Ptr NaiveFitter::create<2>( const Config& c, const dStorm::engine::JobInfo& i );

}
}
