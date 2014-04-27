#include "guf/FitFunctionFactory.h"

#include "guf/FunctionRepository.h"
#include "guf/MultiKernelLambda.h"
#include "threed_info/No3D.h"

namespace dStorm {
namespace guf {

template <typename Assignment, typename Lambda>
inline std::unique_ptr<FitFunctionFactory>
create2( const Config& c, int kernel_count ) 
{ 
    if (kernel_count == 1) {
	typedef typename MultiKernelLambda< 
	    nonlinfit::Bind< Lambda ,Assignment> , 1>
	    ::type F;
	return std::unique_ptr<FitFunctionFactory>( new FunctionRepository<F>(c) );
    } else if (kernel_count == 2) {
	typedef typename MultiKernelLambda< 
	    nonlinfit::Bind< Lambda ,Assignment> , 2>
	    ::type F;
	return std::unique_ptr<FitFunctionFactory>( new FunctionRepository<F>(c) );
    } else {
	throw std::logic_error("Only one or two kernels are supported");
    }
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
        return create2<gaussian_psf::FreeForm,gaussian_psf::No3D>(c,kernel_count);
    else if ( is_no_3d )
        return create2<gaussian_psf::FixedForm,gaussian_psf::No3D>(c,kernel_count);
    else
        return create2<gaussian_psf::FixedForm,gaussian_psf::DepthInfo3D>(c,kernel_count);
}

}
}
