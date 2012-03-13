#ifndef DSTORM_FORMFITTER_H
#define DSTORM_FORMFITTER_H

#include <dStorm/traits/resolution_config.h>
#include "decl.h"
#include <simparm/Structure.hh>
#include "Config.h"
#include <dStorm/engine/Input.h>
#include <dStorm/output/Output.h>
#include <memory>
#include <boost/thread/mutex.hpp>
#include "GUI.h"
#include "Tile.h"
#include "Input.h"
#include <boost/array.hpp>
#include <boost/icl/interval.hpp>
#include <boost/smart_ptr/scoped_ptr.hpp>

namespace dStorm {
namespace form_fitter {
    using namespace output;

    /** This class estimates the standard deviation by averaging
     *  the standard deviations, with a confidence interval for
     *  the mean. */
    class Output : public OutputObject {
      protected:
        boost::mutex mutex;
        
        const form_fitter::Config config;
        dStorm::Engine* engine;
        boost::shared_ptr< const Input > input;
        const bool visual_select;
        typedef boost::ptr_vector<Tile> Tiles;
        Tiles tiles, selected;
        boost::unique_future< Tiles > gui_result;
        std::auto_ptr< FittingVariant > fitter;
        dStorm::traits::resolution::Config result_config;
        boost::array< boost::icl::interval< samplepos::Scalar >::type, 2 > bounds;
        std::vector<bool> seen_fluorophores;
        boost::scoped_ptr<calibrate_3d::ZTruth> z_truth;

        void do_the_fit();

      public:
        typedef simparm::Structure<form_fitter::Config> Config;

        Output(const Config&);
        virtual ~Output();
        Output* clone() const 
            { throw std::runtime_error("Output unclonable."); }

        AdditionalData announceStormSize(const Announcement&);
        RunRequirements announce_run(const RunAnnouncement&) 
            { if ( engine!= NULL ) return RunRequirements().set(MayNeedRestart); else return RunRequirements(); }
        void receiveLocalizations(const EngineResult&);
        void store_results() {}

    };
}
}

#endif
