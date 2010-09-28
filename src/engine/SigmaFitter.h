#ifndef DSTORM_SIGMAFITTER_H
#define DSTORM_SIGMAFITTER_H

#include <memory>
#include <fit++/Exponential2D.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/Localization.h>

namespace dStorm {
namespace engine {

class SigmaFitter {
  protected:
    double initial_sigmas[3], prefac;
    int msx, msy;
    typedef fitpp::Exponential2D::Model<1, 
                                fitpp::Exponential2D::FixedCenter> Fitting;
    Fitting::Constants constants;
    fitpp::FitFunction<Fitting::VarC,false> fit_function;
    std::auto_ptr< Fitting::Fitter<StormPixel>::Type > fitter;
    
  public:
    SigmaFitter(Config &config);
    void useConfig(Config &config);
    /** This function writes sigma_x in deviations[0], sigma_y in
        *  deviations[1], amplitude in deviations[2] and
        *  sigma_xy in deviations[3].
        *  @return true If the free-form fit was sane enough to be used. */
    bool fit(const dStorm::Image<StormPixel,2> &i,
             const Localization &location, double deviations[4]) 
;
    ~SigmaFitter();
  
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif