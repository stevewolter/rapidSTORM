#define DSTORM_SIGMAGUESSER_CPP
#include "SigmaGuesser.h"
#include "Carburettor.h"
#include <CImgBuffer/Image.h>
#include <CImgBuffer/Slot.h>
#include <fit++/Exponential2D.hh>
#include <limits>

#include <cassert>
#include <math.h>

#include "studentPinv.h"
#include "SigmaFitter.h"

using namespace std;
using namespace cimg_library;
using namespace libfitpp;

namespace dStorm {

void (*SigmaGuesser_fitCallback)(double , double, double, int , bool) = NULL;

SigmaGuesserMean::SigmaGuesserMean(Config &c, Carburettor &i) throw()
: ConfigObject("SigmaGuesser", "Standard deviation estimator"),
  config(c), input(i), fitter(new SigmaFitter(c))
{
    sigmas[0] = &c.sigma_x;
    sigmas[1] = &c.sigma_y;
    sigmas[2] = &c.sigma_xy;
    nextCheck = 23;
    deleteAllResults();
}
SigmaGuesserMean::~SigmaGuesserMean() throw() {}

Transmission::Result
SigmaGuesserMean::receiveLocalizations(const EngineResult& er)
    throw()
{
    if (haveInitiatedRestart) return KeepRunning;
    PROGRESS("Adding fits");

    for (int i = 0; i < er.number; i++)
        meanAmplitude.addValue(er.first[i].getStrength());

    PROGRESS("Updated mean amplitude");
    int old_n = n;
    double sigmas[4];
    for (int i = 0; i < er.number; i++) {
        if (meanAmplitude.mean() < er.first[i].getStrength()) {
            PROGRESS("Fitting " << i);
            bool good = fitter->fit(*er.source, er.first[i], sigmas);
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

    if (n >= nextCheck) {
        Transmission::Result r = check();
        nextCheck = 3*nextCheck/2;
#ifndef NDEBUG
        if (SigmaGuesser_fitCallback) {
            if (r == RestartEngine || r == RemoveTransmission)
                SigmaGuesser_fitCallback(config.sigma_x(), config.sigma_y(),
                    config.sigma_xy(), discarded, 
                    ( r == RemoveTransmission) );
        }
#endif
        return r;
    } else
        return KeepRunning;
}

Transmission::Result
SigmaGuesserMean::check() throw() {
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
            PROGRESS("Sigma " << i << " converged at "
                     << (accept[i][0] + accept[i][1])/2);
            converged++;
        } else if ( 
             confInterval[0] > decline[i][1] ||
             confInterval[1] < decline[i][0] )
        {
            double newValue = curVal;

            PROGRESS("Sigma " << i << " changed from " << (*sigmas[i])()
                     << " to " << newValue);
            (*sigmas[i]) = newValue;
                    
            failed++;
        } else {
            PROGRESS("Sigma " << i << " undecided at " << curVal);
        }
    }
    PROGRESS("Checked result");
    if (failed) {
        haveInitiatedRestart = true;
        nextCheck = n+1;
        return RestartEngine;
    } else if (converged == 2) {
        haveInitiatedRestart = true;
        nextCheck = numeric_limits<int>::max();
        return RemoveTransmission;
    } else {
        nextCheck = (3*n)/2;
        return KeepRunning;
    }
}

void SigmaGuesserMean::deleteAllResults() throw() {
    PROGRESS("Resetting entries");
    for (int i = 0; i < 3; i++) {
        double ds = config.delta_sigma() / ((i == 2) ? 10 : 1);
        data[i].reset();
        accept[i][0] = (*sigmas[i])() - ds;
        accept[i][1] = (*sigmas[i])() + ds;
        decline[i][0] = (*sigmas[i])() - 0.5 * ds;
        decline[i][1] = (*sigmas[i])() + 0.5 * ds;
    }
    n = discarded = 0;
    haveInitiatedRestart = false;
    meanAmplitude.reset();

    PROGRESS("Resetting fitter");
    fitter->useConfig(config);
    PROGRESS("Deleted all results");
}

}
