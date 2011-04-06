#ifndef DSTORM_SIGMAGUESSER_H
#define DSTORM_SIGMAGUESSER_H

#include "decl.h"
#include <simparm/Structure.hh>
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

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class Output : public OutputObject {
      protected:
        ost::Mutex mutex;

        boost::shared_ptr< const input::Traits< engine::Image > > traits;
        dStorm::Engine* engine;
        /** Current mean and variance of sigma_x, sigma_y and sigma_xy. */
        Variance<double> data[3];
        /** Acception and rejection intervals around
         *  current values. First index is for x/y/xy,
         *  second for lower/upper bound. */
        double accept[3][2], decline[3][2], delta;
        /** Number of spots used for sigma estimation. */
        int n; 
        /** Necessary spot count for next check. */
        int nextCheck;
        /** Mean amplitude of seen localizations. */
        Mean< amplitude, amplitude::value_type > meanAmplitude;
        /** Total area used for PSF estimation and allowed area. */
        boost::units::quantity<camera::area,int>
            used_area, maximum_area;

        std::auto_ptr<Fitter> fitter;

        int discarded;
        Result check();
        void deleteAllResults();
        void use_traits( const input::Traits<engine::Image>& );

        simparm::StringEntry status;

      public:
        typedef simparm::Structure<sigma_guesser::Config> Config;

        Output(const Config&);
        virtual ~Output();
        Output* clone() const 
            { throw std::runtime_error("Output unclonable."); }

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
