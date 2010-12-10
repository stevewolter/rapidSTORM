#ifndef DSTORM_SIGMAFITTER_H
#define DSTORM_SIGMAFITTER_H

#include <memory>
#include <fit++/Exponential2D.hh>
#include <dStorm/Image.h>
#include <dStorm/Config.h>
#include <dStorm/Localization.h>

namespace dStorm {
class SigmaFitter {
  protected:
    double initial_sigmas[3], prefac;
    int msx, msy;
    std::auto_ptr<libfitpp::Exponential2D_FixedCenter<StormPixel> >
        fitter;
    
  public:
    SigmaFitter(Config &config) throw();
    void useConfig(Config &config) throw();
    /** This function writes sigma_x in deviations[0], sigma_y in
        *  deviations[1], amplitude in deviations[2] and
        *  sigma_xy in deviations[3].
        *  @return true If the free-form fit was sane enough to be used. */
    bool fit(const cimg_library::CImg<StormPixel> &i,
             const Localization &location, double deviations[4]) 
            throw();
  
};
}

#endif
