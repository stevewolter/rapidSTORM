#ifndef DSTORM_SIGMAGUESSER_H
#define DSTORM_SIGMAGUESSER_H

#include <dStorm/units/amplitude.h>
#include "Config.h"
#include <dStorm/engine/Input.h>
#include <dStorm/output/Output.h>
#include <memory>
#include <dStorm/helpers/Variance.h>
#include <dStorm/helpers/thread.h>
#include <boost/units/systems/camera/area.hpp>

namespace dStorm {
namespace sigma_guesser {
    using namespace output;
    class SigmaFitter;

#ifndef DSTORM_SIGMAGUESSER_CPP
    extern void (*SigmaGuesser_fitCallback)
        (double guessX, double guessY, double guessXY,
            int neededSpots, bool converged);
#endif

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class SigmaGuesserMean : public OutputObject {
      protected:
        ost::Mutex mutex;

        boost::shared_ptr< input::Traits< engine::Image > > traits;
        /** Current mean and variance of sigma_x, sigma_y and sigma_xy. */
        Variance<double> data[3];
        /** Acception and rejection intervals around
         *  current values. First index is for x/y/xy,
         *  second for lower/upper bound. */
        double accept[3][2], decline[3][2];
        /** Number of spots used for sigma estimation. */
        int n; 
        /** Necessary spot count for next check. */
        int nextCheck;
        /** Mean amplitude of seen localizations. */
        Mean< amplitude, amplitude::value_type > meanAmplitude;
        /** Total area used for PSF estimation and allowed area. */
        boost::units::quantity<camera::area,int>
            used_area, maximum_area;

        std::auto_ptr<SigmaFitter> fitter;

        Output::Result defined_result;
        int discarded;
        Result check();
        void deleteAllResults();

        simparm::StringEntry status;

      public:
        typedef simparm::Structure<_Config> Config;

        SigmaGuesserMean();
        virtual ~SigmaGuesserMean();
        SigmaGuesserMean* clone() const 
            { throw std::runtime_error("SigmaGuesserMean unclonable."); }

        AdditionalData announceStormSize(const Announcement&);
        RunRequirements announce_run(const RunAnnouncement&) 
            { return RunRequirements().set(MayNeedRestart); }
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal s) {
            if (s == Engine_is_restarted) deleteAllResults();
        }
    };
}
}

#endif
