#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/engine/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/input/Config.h>
#include <memory>
#include <list>
#include <simparm/Set.hh>
#include <simparm/Menu.hh>
#include <simparm/Callback.hh>

namespace dStorm {
    namespace output { class OutputSource; }

    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class Config : public simparm::Set,
        simparm::Listener
    {
      private:
        class TreeRoot;
        std::auto_ptr<input::Config> _inputConfig;
        std::auto_ptr<engine::Config> _engineConfig;
        std::auto_ptr<TreeRoot> outputRoot;
        void registerNamedEntries();
        void operator()(const simparm::Event&);

      public:
        Config();
        Config(const Config &c);
        ~Config();
        Config *clone() const { return new Config(*this); }

        input::Config& inputConfig;
        engine::Config& engineConfig;
        output::OutputSource& outputSource;
        output::Config& outputConfig;

        simparm::Menu helpMenu;
        simparm::Set outputBox;
        simparm::FileEntry configTarget;
        simparm::BoolEntry auto_terminate;
    };
}

#endif
