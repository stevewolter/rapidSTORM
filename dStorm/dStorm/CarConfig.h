#ifndef DSTORM_CARCONFIG_H
#define DSTORM_CARCONFIG_H

#include <dStorm/Image.h>
#include <dStorm/Config.h>
#include <dStorm/TransmissionConfig.h>
#include <dStorm/ViewerConfig.h>
#include <CImgBuffer/Config.h>
#include <memory>

namespace CImgBuffer {
    template <typename T> class Image;
}

namespace dStorm {
    /** Configuration that summarises all
     *  configuration items offered by the dStorm library. */
    class _CarConfig : public ConfigSet
    {
      public:
        CImgBuffer::Config<Image> inputConfig;
        dStorm::Config engineConfig;

        _CarConfig() throw();

        ConfigEntryFile configTarget;
        ConfigEntryBool stderr_progress, printUsedSigma;
    };

    class CarConfig : public _CarConfig {
      private:
        void registerNamedEntries() throw();
        std::auto_ptr<TransmissionSource> transmissionConfigStore;

      public:
        CarConfig(TransmissionSourceFactory& tc_factory) throw();
        CarConfig(const CarConfig &c) throw();

        TransmissionSource& transmissionConfig;
    };
}

#endif
