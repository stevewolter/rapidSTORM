#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include "engine/SpotFinder_decl.h"
#include "engine/SpotFitterFactory_decl.h"
#include "output/Config.h"
#include "input/Config.h"
#include <memory>
#include <list>
#include <simparm/Set.hh>
#include <simparm/Menu.hh>
#include <simparm/Callback.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <boost/ptr_container/ptr_list.hpp>

namespace dStorm {
    namespace output { class OutputSource; }

    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class Config : public simparm::Set
    {
      private:
        class TreeRoot;
        class InputListener;
        class EngineChoice;

        std::auto_ptr<input::Config> _inputConfig;
        std::auto_ptr<TreeRoot> outputRoot;
        std::auto_ptr<InputListener> input_listener;
        std::auto_ptr<EngineChoice> engine_choice;

        void registerNamedEntries();

      public:
        Config();
        Config(const Config &c);
        ~Config();
        Config *clone() const { return new Config(*this); }

        input::Config& inputConfig;
        output::OutputSource& outputSource;
        output::Config& outputConfig;

        simparm::Menu helpMenu;
        simparm::Set outputBox;
        simparm::FileEntry configTarget;
        simparm::BoolEntry auto_terminate;
        /** Number of parallel computation threads to run. */
        simparm::Entry<unsigned long> pistonCount;

        void add_engine( std::auto_ptr<input::chain::Filter> );
        void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> );
        void add_spot_finder( engine::spot_finder::Factory* f ) 
            { add_spot_finder( std::auto_ptr<engine::spot_finder::Factory>(f) ); }
        void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> );
        void add_spot_fitter( engine::spot_fitter::Factory* f ) 
            { add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory>(f) ); }

        const input::chain::MetaInfo& get_meta_info() const;
        std::auto_ptr<input::BaseSource> makeSource();
    };
}

#endif
