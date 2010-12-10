#ifndef DSTORM_SHOCKABSORBERCONFIG_H
#define DSTORM_SHOCKABSORBERCONFIG_H

#include <ConfigSet.hh>

namespace dStorm {
    class ShockAbsorberConfig : public ConfigSet {
      public:
        ConfigEntryFile input;
        ShockAbsorberConfig() throw();

        void registerEntries(ConfigSet &s) throw();
    };
}

#endif
