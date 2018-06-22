#include "debug.h"
#include "guf/NaiveFitter.h"
#include "guf/ModelledFitter.h"
#include "guf/MultiKernelLambda.h"
#include "gaussian_psf/free_form.h"
#include "gaussian_psf/fixed_form.h"
#include "gaussian_psf/expressions.h"
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include "traits/optics.h"
#include "engine/JobInfo.h"
#include "engine/InputTraits.h"
#include <nonlinfit/Bind.h>
#include <boost/utility/in_place_factory.hpp>
#include "threed_info/No3D.h"
#include "guf/EvaluationTags.h"

namespace dStorm {
namespace guf {

struct add_fit_window_width {
    typedef void result_type;
    template <int ChunkSize, typename P1, typename P2>
    void operator()( nonlinfit::plane::Disjoint<float,ChunkSize,P1,P2> t, bool disjoint, bool double_computation, std::set<int>& target ) {
        if (disjoint && !double_computation) {
            target.insert(ChunkSize);
        }
    }

    template <int ChunkSize, typename P1, typename P2>
    void operator()( nonlinfit::plane::Disjoint<double,ChunkSize,P1,P2> t, bool disjoint, bool double_computation, std::set<int>& target ) {
        if (disjoint && double_computation) {
            target.insert(ChunkSize);
        }
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    void operator()( nonlinfit::plane::Joint<Num,ChunkSize, P1, P2> t, bool disjoint, bool double_computation, std::set<int>& target ) {
    }
};

std::set<int> desired_fit_window_widths(const Config& config) {
    std::set<int> result;
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( add_fit_window_width(), _1, config.allow_disjoint(), config.double_computation(), boost::ref(result)));
    return result;
}

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
    else if ( c.free_sigmas() )
        return create2<Kernels,gaussian_psf::FreeForm,gaussian_psf::No3D>(c,info);
    else if ( consistently_no_3d )
        return create2<Kernels,gaussian_psf::FixedForm,gaussian_psf::No3D>(c,info);
    else
        return create2<Kernels,gaussian_psf::FixedForm,gaussian_psf::DepthInfo3D>( c, info );
}

template NaiveFitter::Ptr NaiveFitter::create<1>( const Config& c, const dStorm::engine::JobInfo& i );
template NaiveFitter::Ptr NaiveFitter::create<2>( const Config& c, const dStorm::engine::JobInfo& i );

}
}
