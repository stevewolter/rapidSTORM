#ifndef DSTORM_SIGMAGUESSER_H
#define DSTORM_SIGMAGUESSER_H

#include "engine/Config.h"
#include "Output.h"
#include <memory>
#include "statistics/Variance.h"

namespace dStorm {
    class Input;
    class SigmaFitter;

#ifndef DSTORM_SIGMAGUESSER_CPP
    extern void (*SigmaGuesser_fitCallback)
        (double guessX, double guessY, double guessXY,
            int neededSpots, bool converged);
#endif

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class SigmaGuesserMean : public Output, public simparm::Object {
      protected:
        ost::Mutex mutex;

        Config &config;
        Input &input;
        DoubleEntry *sigmas[3];
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
        Result check();
        void deleteAllResults();

        simparm::StringEntry status;

      public:
        SigmaGuesserMean( Config &config, Input &input );
        virtual ~SigmaGuesserMean();
        SigmaGuesserMean* clone() const 
            { throw std::runtime_error("SigmaGuesserMean unclonable."); }

        AdditionalData announceStormSize(const Announcement&) 
            { return SourceImage; }
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal s) {
            if (s == Engine_is_restarted) deleteAllResults();
        }
    };
}

#endif
