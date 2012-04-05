#include "debug.h"
#include "NaiveFitter.h"
#include "ModelledFitter.h"
#include "guf/psf/StandardFunction.h"
#include "guf/psf/free_form.h"
#include "guf/psf/fixed_form.h"
#include "guf/psf/expressions.h"
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <dStorm/traits/optics.h>
#include <dStorm/engine/JobInfo.h>
#include <dStorm/engine/InputTraits.h>
#include <nonlinfit/Bind.h>
#include "../select_3d_lambda.hpp"

namespace dStorm {
namespace guf {

template <int Kernels, typename Assignment>
class creator3
: public boost::static_visitor< NaiveFitter::Ptr >
{
  public:
    template <typename Traits3D>
    NaiveFitter::Ptr 
    operator()( const Config& c, const dStorm::engine::JobInfo& i, 
                const Traits3D t3d ) const
    { 
        typedef typename PSF::StandardFunction< 
            nonlinfit::Bind< typename select_3d_lambda<Traits3D>::type ,Assignment> ,Kernels>
            ::type F;
        DEBUG("Model has " << boost::mpl::size<typename F::Variables>::type::value << " variables from "
              << "original kernel's " << boost::mpl::size<typename select_3d_lambda<Traits3D>::type::Variables>::type::value);
        return std::auto_ptr<NaiveFitter>( new ModelledFitter<F>(c,i) );
    }
};

template <int Kernels, typename Assignment>
inline NaiveFitter::Ptr
create1( 
    const Config& c, 
    const dStorm::engine::JobInfo& i )
{
    return boost::apply_visitor( 
        boost::bind( creator3<Kernels,Assignment>(), boost::cref(c), boost::cref(i), _1 ),
        *i.traits.optics(0).depth_info() );
}

template <>
inline NaiveFitter::Ptr
create1<2,PSF::FreeForm>( const Config& c, const dStorm::engine::JobInfo& i )
{
    throw std::logic_error("Two kernels and free form can't be instantiated "
                           "on 32 bit machines");
}


template <int Kernels>
NaiveFitter::Ptr
NaiveFitter::create(
    const Config& c, 
    const dStorm::engine::JobInfo& i )
{
    if ( c.free_sigmas() )
        return create1<Kernels,PSF::FreeForm>(c,i);
    else 
        return create1<Kernels,PSF::FixedForm>(c,i);
}

template NaiveFitter::Ptr NaiveFitter::create<1>( const Config& c, const dStorm::engine::JobInfo& i );
template NaiveFitter::Ptr NaiveFitter::create<2>( const Config& c, const dStorm::engine::JobInfo& i );

}
}
