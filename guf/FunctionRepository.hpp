#ifndef GUF_EVALUATOR_FACTORY_IMPL_H
#define GUF_EVALUATOR_FACTORY_IMPL_H

#include <Eigen/StdVector>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

#include "fit_window/Optics.h"
#include "fit_window/Plane.h"
#include "gaussian_psf/is_plane_dependent.h"
#include "guf/create_evaluators.h"
#include "guf/EvaluationTags.h"
#include "guf/FunctionRepository.h"
#include "LengthUnit.h"
#include "nonlinfit/make_bitset.h"
#include "nonlinfit/plane/Joint.h"
#include "nonlinfit/plane/JointData.h"
#include "nonlinfit/plane/Disjoint.h"
#include "nonlinfit/plane/DisjointData.h"

#include "debug.h"

namespace dStorm {
namespace guf {

template < class Function >
struct FunctionRepository<Function>::instantiate
{
    typedef void result_type;

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Disjoint<Num,ChunkSize,P1,P2> t,
        int width,
        bool disjoint,
        bool use_doubles
    ) const { 
        return disjoint && ChunkSize == width &&
            (boost::is_same<Num,float>::value || use_doubles);
    }

    template <int ChunkSize, typename Num, typename P1, typename P2>
    bool is_appropriate( 
        nonlinfit::plane::Joint<Num,ChunkSize,P1,P2>,
        int,
        bool disjoint,
        bool use_doubles
    ) const { 
        return (boost::is_same<Num,float>::value || use_doubles); 
    }

    /** Instantiate a Function wrapped by the MetaFunction computed by Tag.
     *  The instantiated function is stored in the supplied target store,
     *  at the index of the given tag in the instantiation schedule. */
    template <typename Tag, typename Container>
    void operator()( Tag way, Function& expression, const fit_window::Plane& data, bool disjoint, bool use_doubles, bool mle, int width, Container& target )
    {
        if (target.get() == nullptr && is_appropriate(way, width, disjoint, use_doubles)) {
            target.reset(PlaneFunction<Tag>::create(create_evaluators(expression, way), data, mle).release());
        }
    }
};

template <class Function>
FunctionRepository<Function>::FunctionRepository(const Config& config) 
: expression( new Function() ), disjoint(config.allow_disjoint()),
  use_doubles(config.double_computation()),
  disjoint_amplitudes(config.disjoint_amplitudes()),
  laempi_fit(config.laempi_fit()),
  model(*expression) {
    for (gaussian_psf::BaseExpression& gaussian : model) {
        gaussian.set_negligible_step_length(ToLengthUnit(config.negligible_x_step()));
        gaussian.set_relative_epsilon(config.relative_epsilon());
    }
    model.background_model().set_relative_epsilon(config.relative_epsilon());
}

template <class Function>
FunctionRepository<Function>::~FunctionRepository() 
{}

template <class Function>
std::vector<bool> FunctionRepository<Function>::reduction_bitset() const OVERRIDE {
    return nonlinfit::make_bitset( 
        typename Function::Variables(), 
        gaussian_psf::is_plane_independent( laempi_fit, disjoint_amplitudes ) );
}

template <class Function>
std::unique_ptr<typename FunctionRepository<Function>::result_type>
FunctionRepository<Function>::create_function( const fit_window::Plane& data, bool mle )
{
    std::unique_ptr<result_type> result;
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( instantiate(),
                     _1, boost::ref(*expression), boost::ref(data), disjoint, use_doubles, mle, data.window_width, boost::ref(result) ) );
    assert(result);
    return result;
}

}
}

#endif
