#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/input/Config.h>
#include <memory>
#include <simparm/Set.hh>

namespace dStorm {
namespace output { class OutputSource; }
namespace engine {
    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class CarConfig : public simparm::Object {
      private:
        class TreeRoot;
        std::auto_ptr<input::Config> _inputConfig;
        std::auto_ptr<engine::Config> _engineConfig;
        std::auto_ptr<TreeRoot> outputRoot;
        void registerNamedEntries();

      public:
        CarConfig();
        CarConfig(const CarConfig &c);
        ~CarConfig();
        CarConfig *clone() const { return new CarConfig(*this); }

        input::Config& inputConfig;
        engine::Config& engineConfig;
        output::OutputSource& outputSource;
        output::Config& outputConfig;

        simparm::Set outputBox;
        simparm::FileEntry configTarget;
    };
}
}

#endif
