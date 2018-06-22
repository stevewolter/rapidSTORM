#ifndef GUF_EVALUATOR_FACTORY_H
#define GUF_EVALUATOR_FACTORY_H

#include <boost/ptr_container/ptr_vector.hpp>
#include <memory>
#include <boost/utility.hpp>
#include "guf/Config.h"
#include "guf/FitFunctionFactory.h"
#include "guf/PlaneFunction.h"
#include "fit_window/Spot.h"
#include "constant_background/model.hpp"

namespace dStorm {
namespace guf {

/** Generator for an evaluating function for each tag in the instantiation 
 *  schedule. 
 *  This class is responsible for generating and storing one instance of 
 *  nonlinfit::plane::Distance per tag in the instantiation schedule. It also stores an instance of the function's
 *  base expression shared between all functions, which can be accessed by 
 *  get_expression(). */
template <class Kernel, class DataTagList>
class FitFunctionFactoryImplementation : public FitFunctionFactory, private boost::noncopyable
{
  private:
    class instantiate;

    /** The expression is dynamically allocated to avoid Eigen alignment trouble. */
    std::vector<std::unique_ptr<Kernel>> kernels;
    std::unique_ptr<constant_background::Expression> background;
    bool disjoint_amplitudes, laempi_fit, use_background;
    MultiKernelModel model;

  public:
    FitFunctionFactoryImplementation(const Config& config, int kernel_count, bool use_background);
    std::vector<bool> reduction_bitset() const OVERRIDE;
    MultiKernelModel fit_position() OVERRIDE { return const_cast<const MultiKernelModel&>(model); }
    /** Return an abstract function with the expression set to the result of
     *  get_expression() and the data to the supplied data. If \c mle is true,
     *  the function will be an instance of 
     *  nonlinfit::plane::InversePoissonLikelihood, and of 
     *  nonlinfit::plane::SquaredDeviations otherwise.
     **/
    std::unique_ptr<FitFunction> create_function( const fit_window::Plane&, bool mle ) OVERRIDE;

    Kernel& get_gaussian() { assert(kernels.size() == 1); return *kernels[0]; }
    constant_background::Expression& get_background() { return *background; }
    Spot get_center() const {
        Spot result;
        result.x() = (*kernels[0])( gaussian_psf::Mean<0>() );
        result.y() = (*kernels[0])( gaussian_psf::Mean<1>() );
        return result;
    }
    Spot get_width() const { return kernels[0]->get_sigma(); }
    double get_constant_background() const {
        if (use_background) {
            return (*background)( constant_background::Amount() );
        } else {
            return 0;
        }
    }
};

}
}

#endif
