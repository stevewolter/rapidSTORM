#include "guf/FitFunctionFactory.h"

#include "config.h"
#include "gaussian_psf/DepthInfo3D.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/free_form.h"
#include "gaussian_psf/No3D.h"
#include "guf/FitFunctionFactoryImplementation.hpp"
#include "nonlinfit/Bind.h"
#include "threed_info/No3D.h"

namespace dStorm {
namespace guf {

/** Prioritized lists of nonlinfit data tags for fitting functions. */
typedef boost::mpl::vector<
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,20>::type,
    nonlinfit::plane::xs_disjoint<double,18>::type,
    nonlinfit::plane::xs_disjoint<double,16>::type,
#endif
    nonlinfit::plane::xs_disjoint<double,14>::type,
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<double,12>::type,
    nonlinfit::plane::xs_disjoint<double,10>::type,
    nonlinfit::plane::xs_disjoint<double,8>::type,
#endif
    nonlinfit::plane::xs_joint<double,8>::type
> disjoint_double_tags;

typedef boost::mpl::vector<
    nonlinfit::plane::xs_joint<double,8>::type
> no_disjoint_double_tags;

typedef boost::mpl::vector<
#if defined(USE_SPECIALIZED_FITTERS)
    nonlinfit::plane::xs_disjoint<float,20>::type,
    nonlinfit::plane::xs_disjoint<float,16>::type,
    nonlinfit::plane::xs_disjoint<float,12>::type,
    nonlinfit::plane::xs_disjoint<float,8>::type,
#endif
    nonlinfit::plane::xs_joint<float,8>::type
> disjoint_float_tags;

typedef boost::mpl::vector<
    nonlinfit::plane::xs_joint<float,8>::type
> no_disjoint_float_tags;

struct add_fit_window_width {
    typedef void result_type;
    template <typename Number, int ChunkSize, typename P1, typename P2>
    void operator()( nonlinfit::plane::Disjoint<Number,ChunkSize,P1,P2> t, std::set<int>& target ) {
        target.insert(ChunkSize);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    void operator()( nonlinfit::plane::Joint<Num,ChunkSize, P1, P2> t, std::set<int>& target ) {
    }
};

template <typename Tags>
std::set<int> desired_fit_window_widths(Tags tags) {
    std::set<int> result;
    boost::mpl::for_each< Tags >(
        boost::bind( add_fit_window_width(), _1, boost::ref(result)));
    return result;
}

std::set<int> desired_fit_window_widths(const Config& config) {
    if (config.allow_disjoint()) {
        if (config.double_computation()) {
            return desired_fit_window_widths(disjoint_double_tags());
        } else {
            return desired_fit_window_widths(disjoint_float_tags());
        }
    } else {
        if (config.double_computation()) {
            return desired_fit_window_widths(no_disjoint_double_tags());
        } else {
            return desired_fit_window_widths(no_disjoint_float_tags());
        }
    }
}

template <typename Assignment, typename Lambda, typename DataTags>
inline std::unique_ptr<FitFunctionFactory>
create2( const Config& c, int kernel_count, bool use_background ) 
{ 
    typedef nonlinfit::Bind< Lambda ,Assignment> F;
    return std::unique_ptr<FitFunctionFactory>( new FitFunctionFactoryImplementation<F, DataTags>(c, kernel_count, use_background) );
}

template <typename Assignment, typename Lambda>
inline std::unique_ptr<FitFunctionFactory>
create1( const Config& c, int kernel_count, bool use_background ) {
    if (c.allow_disjoint()) {
        if (c.double_computation()) {
            return create2<Assignment, Lambda, disjoint_double_tags>(c, kernel_count, use_background);
        } else {
            return create2<Assignment, Lambda, disjoint_float_tags>(c, kernel_count, use_background);
        }
    } else {
        if (c.double_computation()) {
            return create2<Assignment, Lambda, no_disjoint_double_tags>(c, kernel_count, use_background);
        } else {
            return create2<Assignment, Lambda, no_disjoint_float_tags>(c, kernel_count, use_background);
        }
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
        return create1<gaussian_psf::FreeForm,gaussian_psf::No3D>(c,kernel_count, !plane.has_background_estimate);
    else if ( is_no_3d )
        return create1<gaussian_psf::FixedForm,gaussian_psf::No3D>(c,kernel_count, !plane.has_background_estimate);
    else
        return create1<gaussian_psf::FixedForm,gaussian_psf::DepthInfo3D>(c,kernel_count, !plane.has_background_estimate);
}

}
}
