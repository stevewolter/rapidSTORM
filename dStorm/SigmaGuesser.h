#ifndef DSTORM_SIGMAGUESSER_H
#define DSTORM_SIGMAGUESSER_H

#include <dStorm/Config.h>
#include <dStorm/Transmission.h>
#include <CImg.h>
#include <memory>
#include "Variance.h"

namespace dStorm {
    class Carburettor;
    class SigmaFitter;

#ifndef DSTORM_SIGMAGUESSER_CPP
    extern void (*SigmaGuesser_fitCallback)
        (double guessX, double guessY, double guessXY,
            int neededSpots, bool converged);
#endif

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class SigmaGuesserMean : public Transmission {
      protected:
        Config &config;
        Carburettor &input;
        ConfigEntryDouble *sigmas[3];
        /** Current mean and variance of sigma_x, sigma_y and sigma_xy. */
        Variance data[3];
        /** Acception and rejection intervals around
         *  current values. First index is for x/y/xy,
         *  second for lower/upper bound. */
        double accept[3][2], decline[3][2];
        /** Number of spots used for sigma estimation. */
        int n; 
        /** Necessary spot count for next check. */
        int nextCheck;
        /** Mean amplitude of seen localizations. */
        Variance meanAmplitude;

        std::auto_ptr<SigmaFitter> fitter;

        bool haveInitiatedRestart;
        int discarded;
        Result check() throw();

      public:
        SigmaGuesserMean( Config &config, Carburettor &input ) throw();
        virtual ~SigmaGuesserMean() throw();

        AdditionalData announceStormSize(int, int, int) throw() 
            { return SourceImage; }
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw();
        void finish() throw() {}
        const char *getName() throw() { return "SigmaGuesser"; }
    };
}

#endif
