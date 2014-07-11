#ifndef DSTORM_FORMFITTER_H
#define DSTORM_FORMFITTER_H

#include <array>
#include <memory>

#include <boost/icl/interval.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "engine/Input.h"
#include "estimate_psf_form/Config.h"
#include "estimate_psf_form/decl.h"
#include "estimate_psf_form/GUI.h"
#include "estimate_psf_form/Input.h"
#include "estimate_psf_form/Tile.h"
#include "output/Output.h"
#include "simparm/ProgressEntry.h"
#include "traits/optics_config.h"

namespace dStorm {
namespace estimate_psf_form {
    using namespace output;

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class Output : public output::Output {
      protected:
        simparm::NodeHandle current_ui;
        boost::mutex mutex;
        
        const estimate_psf_form::Config config;
        dStorm::Engine* engine;
        boost::shared_ptr< const Input > input;
        const bool visual_select;
        typedef boost::ptr_vector<Tile> Tiles;
        Tiles tiles, selected;
        boost::unique_future< Tiles > gui_result;
        std::auto_ptr< FittingVariant > fitter;
        dStorm::traits::MultiPlaneConfig result_config;
        std::array< boost::icl::interval< samplepos::Scalar >::type, 2 > bounds;
        std::vector<bool> seen_fluorophores;
        double current_limit;
        simparm::ProgressEntry collection, fit;

        void do_the_fit();
        void attach_ui_( simparm::NodeHandle );

      public:
        Output(const Config&);
        virtual ~Output();
        Output* clone() const 
            { throw std::runtime_error("Output unclonable."); }

        void announceStormSize(const Announcement&) OVERRIDE;
        RunRequirements announce_run(const RunAnnouncement&) 
            { if ( engine!= NULL ) return RunRequirements().set(MayNeedRestart); else return RunRequirements(); }
        void receiveLocalizations(const EngineResult&);
    };
}
}

#endif
