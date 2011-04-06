#ifndef DSTORM_SIGMAFITTER_H
#define DSTORM_SIGMAFITTER_H

#include <memory>
#include <fit++/Exponential2D.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/Localization.h>
#include "Config.h"

namespace dStorm {
namespace sigma_guesser {

class Fitter {
  protected:
    double initial_sigmas[3], prefac;
    int msx, msy;
    typedef fitpp::Exponential2D::Model<1, 
                                fitpp::Exponential2D::FixedCenter> Fitting;
    typedef Fitting::Fitter<engine::StormPixel, Eigen::Dynamic, Eigen::Dynamic, 1>::Type _Fitter;
    Fitting::Constants constants;
    fitpp::FitFunction<Fitting::VarC,false> fit_function;
    std::auto_ptr< _Fitter > fitter;
    
  public:
    Fitter(const Config &config);
    void useConfig(const input::Traits<engine::Image>& traits);
    /** This function writes sigma_x in deviations[0], sigma_y in
        *  deviations[1], amplitude in deviations[2] and
        *  sigma_xy in deviations[3].
        *  @return true If the free-form fit was sane enough to be used. */
    bool fit(const dStorm::Image<engine::StormPixel,2> &i,
             const Localization &location, double deviations[4]);
    ~Fitter();
  
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
