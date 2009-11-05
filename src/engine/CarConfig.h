#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/engine/Image.h>
#include <dStorm/engine/Config.h>
#include <dStorm/output/OutputSource.h>
#include <dStorm/output/OutputFactory.h>
#include <dStorm/input/Config.h>
#include <memory>
#include <simparm/Set.hh>

namespace CImgBuffer {
    template <typename T> class Image;
}

namespace dStorm {
    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class CarConfig : public simparm::Object {
      private:
        std::auto_ptr<CImgBuffer::Config> _inputConfig;
        std::auto_ptr<dStorm::Config> _engineConfig;
        std::auto_ptr<OutputSource> outputRoot;
        void registerNamedEntries();

      public:
        CarConfig(OutputFactory& tc_factory);
        CarConfig(const CarConfig &c);
        ~CarConfig();
        CarConfig *clone() const { return new CarConfig(*this); }

        CImgBuffer::Config& inputConfig;
        dStorm::Config& engineConfig;
        OutputSource& outputConfig;

        simparm::Set outputBox;
        FileEntry configTarget;
    };
}

#endif
