#include "guf/FitFunctionFactory.h"

#include "guf/FitFunctionFactoryImplementation.hpp"
#include "threed_info/No3D.h"

#include "nonlinfit/Bind.h"
#include "gaussian_psf/No3D.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/free_form.h"
#include "gaussian_psf/fixed_form.h"

namespace dStorm {
namespace guf {

template <typename Assignment, typename Lambda>
inline std::unique_ptr<FitFunctionFactory>
create2( const Config& c, int kernel_count, bool use_background ) 
{ 
    typedef nonlinfit::Bind< Lambda ,Assignment> F;
    return std::unique_ptr<FitFunctionFactory>( new FitFunctionFactoryImplementation<F, constant_background::Expression>(c, kernel_count, use_background) );
}
 
std::unique_ptr<FitFunctionFactory>
FitFunctionFactory::create( 
    const Config& c, 
    const engine::InputPlane& plane,
    int kernel_count)
{
    bool is_no_3d = dynamic_cast< const threed_info::No3D* >( plane.optics.depth_info(Direction_X).get() );
    assert(is_no_3d == (dynamic_cast< const threed_info::No3D* >(plane.optics.depth_info(Direction_Y).get()) != nullptr));

    if ( c.free_sigmas() && ! is_no_3d )
        throw std::runtime_error("Free-sigma fitting is limited to 2D");
    else if ( c.free_sigmas() )
        return create2<gaussian_psf::FreeForm,gaussian_psf::No3D>(c,kernel_count, !plane.has_background_estimate);
    else if ( is_no_3d )
        return create2<gaussian_psf::FixedForm,gaussian_psf::No3D>(c,kernel_count, !plane.has_background_estimate);
    else
        return create2<gaussian_psf::FixedForm,gaussian_psf::DepthInfo3D>(c,kernel_count, !plane.has_background_estimate);
}

}
}
