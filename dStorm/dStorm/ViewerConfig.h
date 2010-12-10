#ifndef DSTORM_VIEWERCONFIG_H
#define DSTORM_VIEWERCONFIG_H

#include <ConfigSet.hh>
#include <dStorm/Config.h>

namespace dStorm {
    class ViewerConfig : public virtual ConfigSet {
      public:
        ConfigEntryBool showOutput;
        ConfigEntryFile tifFile;
        ConfigEntryDouble res_enh;
        ConfigEntryUnsignedLong refreshCycle;
        ConfigEntryDouble amplitudeThreshold;
        ConfigEntryDouble histogramPower;
        ConfigEntryLong zoom;
        ConfigEntryChoice colourScheme;
        ConfigEntryTrigger save, quit;

        ViewerConfig();

        void registerEntries(ConfigSet &at) throw();
    };
}

#endif
