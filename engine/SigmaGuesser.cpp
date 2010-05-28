#define DSTORM_SIGMAGUESSER_CPP
#include "engine/SigmaGuesser.h"
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Image.h>
#include <fit++/Exponential2D.hh>
#include <limits>
#include <boost/units/io.hpp>

#include <cassert>
#include <math.h>

#include "studentPinv.h"
#include "engine/SigmaFitter.h"

#include <sstream>

using namespace std;
using namespace cimg_library;
using namespace fitpp;

namespace dStorm {
namespace engine {

void (*SigmaGuesser_fitCallback)(double , double, double, int , bool) = NULL;

SigmaGuesserMean::SigmaGuesserMean(Config &c)
: OutputObject("SigmaGuesser", "Standard deviation estimator"),
  config(c), fitter(new SigmaFitter(c)),
  status("Status", "Std. dev. estimation")
{
    nextCheck = 23;
    deleteAllResults();

    stringstream ss;
    ss << "Trying " << c.sigma_x() << ", " << c.sigma_y() << 
          " with correlation " << c.sigma_xy();
    status = ss.str();
    status.editable = false;

    push_back( status );
}
SigmaGuesserMean::~SigmaGuesserMean() {}

Output::Result
SigmaGuesserMean::receiveLocalizations(const EngineResult& er)

{
    if (defined_result != KeepRunning) return defined_result;
    ost::MutexLock lock(mutex);
    if (defined_result != KeepRunning) return defined_result;
    PROGRESS("Adding fits");

    for (int i = 0; i < er.number; i++)
        meanAmplitude.addValue(er.first[i].getStrength());

    PROGRESS("Updated mean amplitude");
    int old_n = n;
    double sigmas[4];
    for (int i = 0; i < er.number; i++) {
        if (meanAmplitude.mean() < er.first[i].getStrength()) {
            PROGRESS("Fitting " << i);
            cimg_library::CImg<StormPixel> srcim(
                er.source->ptr(), 
                er.source->width() / cs_units::camera::pixel,
                er.source->height() / cs_units::camera::pixel,
                1,
                1,
                true );
            bool good = fitter->fit(srcim, er.first[i], sigmas);
            PROGRESS("Fitted " << i);
            if (good) {
                for (int i = 0; i < 3; i++) {
                    PROGRESS("Guess for " << i << " is " << sigmas[i+((i==2)?1:0)]);
                    data[i].addValue(sigmas[i+((i==2)?1:0)]);
                }
                n++;
            } 
        }
    }
    discarded += n - old_n;
    PROGRESS("Fitted data");

    PROGRESS("Have " << n << " of " << nextCheck);
    if (n >= nextCheck) {
        Output::Result r = check();
        nextCheck = 3*nextCheck/2;
#ifndef NDEBUG
        if (SigmaGuesser_fitCallback) {
            if (r == RestartEngine || r == RemoveThisOutput)
                SigmaGuesser_fitCallback(
                    config.sigma_x() / cs_units::camera::pixel,
                    config.sigma_y() / cs_units::camera::pixel,
                    config.sigma_xy(), discarded, 
                    ( r == RemoveThisOutput) );
        }
#endif
        return r;
    } else
        return KeepRunning;
}

Output::Result
SigmaGuesserMean::check() {
    PROGRESS("Checking result");
    int converged = 0, failed = 0;
    double t_term = studentPinv95(n-1);
    for (int i = 0; i < 3; i++) {
        double confidence = t_term * sqrt( data[i].variance() / n );
        double confInterval[2];
        double curVal = data[i].mean();
        confInterval[0] = curVal - confidence, confInterval[1] = curVal + confidence;

        if ( (confInterval[0] >= accept[i][0] &&
              confInterval[1] <= accept[i][1]) ) 
        {
            STATUS("Sigma " << i << " converged at "
                     << (accept[i][0] + accept[i][1])/2);
            converged++;
        } else if ( 
             confInterval[0] > decline[i][1] ||
             confInterval[1] < decline[i][0] )
        {
            double newValue = curVal;

            STATUS("Sigma " << i << " changed from " << (*sigmas[i])()
                     << " to " << newValue);
            if ( i == 0 )
                config.sigma_x = float(newValue) * cs_units::camera::pixel;
            else if ( i == 1 )
                config.sigma_y = float(newValue) * cs_units::camera::pixel;
            else
                config.sigma_xy = newValue;
                    
            failed++;
        } else {
            STATUS("Sigma " << i << " undecided at " << curVal);
        }
    }
    PROGRESS("Checked result");
    if (failed) {
        stringstream ss;
        ss << "Trying " << config.sigma_x() << ", " << config.sigma_y() << 
            " with correlation " << config.sigma_xy();
        status = ss.str();

        defined_result = RestartEngine;
        nextCheck = n+1;
        STATUS("Significant difference");
    } else if (converged == 3) {
        nextCheck = numeric_limits<int>::max();
        STATUS("Insignificant difference");
        defined_result = RemoveThisOutput;
    } else {
        nextCheck = (3*n)/2;
        STATUS("No significance. Next check at " << nextCheck);
    }

    return defined_result;
}

void SigmaGuesserMean::deleteAllResults() {
    ost::MutexLock lock(mutex);
    PROGRESS("Resetting entries");
    double sigmas[3] = {
        config.sigma_x() / cs_units::camera::pixel,
        config.sigma_y() / cs_units::camera::pixel,
        config.sigma_xy() };
    for (int i = 0; i < 3; i++) {
        double ds = config.delta_sigma() / ((i == 2) ? 2 : 1);
        data[i].reset();
        accept[i][0] = sigmas[i] - ds;
        accept[i][1] = sigmas[i] + ds;
        decline[i][0] = sigmas[i] - 0.5 * ds;
        decline[i][1] = sigmas[i] + 0.5 * ds;
    }
    n = discarded = 0;
    defined_result = KeepRunning;
    meanAmplitude.reset();

    PROGRESS("Resetting fitter");
    fitter->useConfig(config);
    PROGRESS("Deleted all results");
}

}
}