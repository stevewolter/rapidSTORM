#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/Image.h>
#include <dStorm/Config.h>
#include <dStorm/OutputSource.h>
#include <dStorm/OutputFactory.h>
#include <CImgBuffer/Config.h>
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

        CImgBuffer::Config& inputConfig;
        dStorm::Config& engineConfig;
        OutputSource& outputConfig;

        simparm::Set outputBox;
        FileEntry configTarget;
    };
}

#endif
