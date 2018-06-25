#ifndef GUF_EVALUATOR_FACTORY_IMPL_H
#define GUF_EVALUATOR_FACTORY_IMPL_H

#include <Eigen/StdVector>
#include "FunctionRepository.h"
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>
#include "EvaluationTags.h"
#include "fit_window/Plane.h"

#include "debug.h"

namespace dStorm {
namespace guf {

template < class Function >
struct FunctionRepository<Function>::instantiate
{
    typedef void result_type;
    /** Instantiate a Function wrapped by the MetaFunction computed by Tag.
     *  The instantiated function is stored in the supplied target store,
     *  at the index of the given tag in the instantiation schedule. */
    template <typename Tag, typename Container>
    void operator()( Tag way, Function& expression, Container& target )
    {
        target.push_back( PlaneFunction<Function>::create(expression, way) );
    }
};

template <class Function>
FunctionRepository<Function>::FunctionRepository() 
: expression( new Function() ),
  mover( new nonlinfit::AbstractedMoveable<nonlinfit::VectorPosition<Function> >(*expression) )
{
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( instantiate(),
                     boost::arg<1>(), boost::ref(*expression), boost::ref(store) ) );
}

template <class Function>
FunctionRepository<Function>::~FunctionRepository() 
{}

template <class Function>
typename FunctionRepository<Function>::result_type*
FunctionRepository<Function>::operator()( const fit_window::Plane& data, bool mle )
{
    const int index = data.tag_index; 
    return &store[index].for_data( data, (mle) ? PoissonLikelihood : LeastSquares );
}

}
}

#endif
