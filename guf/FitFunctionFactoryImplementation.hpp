#ifndef GUF_EVALUATOR_FACTORY_IMPL_H
#define GUF_EVALUATOR_FACTORY_IMPL_H

#include <Eigen/StdVector>
#include <boost/mpl/for_each.hpp>
#include <boost/bind/bind.hpp>

#include "fit_window/Optics.h"
#include "fit_window/Plane.h"
#include "gaussian_psf/is_plane_dependent.h"
#include "guf/create_evaluator.h"
#include "guf/EvaluationTags.h"
#include "guf/FitFunctionFactoryImplementation.h"
#include "LengthUnit.h"
#include "nonlinfit/make_bitset.h"
#include "nonlinfit/plane/Joint.h"
#include "nonlinfit/plane/JointData.h"
#include "nonlinfit/plane/Disjoint.h"
#include "nonlinfit/plane/DisjointData.h"

#include "debug.h"

namespace dStorm {
namespace guf {

template <class Kernel>
struct FitFunctionFactoryImplementation<Kernel>::instantiate
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
    void operator()( Tag way, FitFunctionFactoryImplementation& repository, const fit_window::Plane& data, bool mle, Container& target )
    {
        if (target.get() == nullptr && is_appropriate(way, data.window_width, repository.disjoint, repository.use_doubles)) {
            std::vector<std::unique_ptr<nonlinfit::plane::Term<Tag>>> evaluators;
            for (auto& kernel : repository.kernels) {
                evaluators.push_back(create_evaluator(*kernel, way));
            }
            if (repository.use_background) {
                evaluators.push_back(create_evaluator(*repository.background, way));
            }
            target.reset(PlaneFunction<Tag>::create(std::move(evaluators), data, mle).release());
        }
    }
};

template <class Kernel>
FitFunctionFactoryImplementation<Kernel>::FitFunctionFactoryImplementation(
    const Config& config, int kernel_count, bool use_background) 
: disjoint(config.allow_disjoint()),
  use_doubles(config.double_computation()),
  disjoint_amplitudes(config.disjoint_amplitudes()),
  laempi_fit(config.laempi_fit()),
  use_background(use_background) {
    for (int i = 0; i < kernel_count; ++i) {
	std::unique_ptr<Kernel> kernel(new Kernel());
        kernel->set_negligible_step_length(ToLengthUnit(config.negligible_x_step()));
        kernel->set_relative_epsilon(config.relative_epsilon());
        model.add_kernel(kernel.get());
	kernels.push_back(std::move(kernel));
    }
    background.reset(new constant_background::Expression());
    background->set_relative_epsilon(config.relative_epsilon());
    model.set_constant(background.get());
}

template <class Kernel>
std::vector<bool> FitFunctionFactoryImplementation<Kernel>::reduction_bitset() const {
    std::vector<bool> result;
    std::vector<bool> kernel_set = nonlinfit::make_bitset( 
        typename Kernel::Variables(), 
        gaussian_psf::is_plane_independent( laempi_fit, disjoint_amplitudes ) );

    for (size_t i = 0; i <  kernels.size(); ++i) {
        std::copy(kernel_set.begin(), kernel_set.end(), std::back_inserter(result));
    }
    if (use_background) {
        result.push_back(false);
    }

    return result;
}

template <class Kernel>
std::unique_ptr<nonlinfit::AbstractFunction<double>>
FitFunctionFactoryImplementation<Kernel>::create_function( const fit_window::Plane& data, bool mle )
{
    assert(data.has_per_pixel_background || use_background);
    std::unique_ptr<result_type> result;
    boost::mpl::for_each< evaluation_tags >( 
        boost::bind( instantiate(),
                     _1, boost::ref(*this), boost::ref(data), mle, boost::ref(result) ) );
    assert(result);
    return result;
}

}
}

#endif
