#include <ConfigEntry.hh>
#include <ConfigSet.hh>

namespace AndorCamera {
    using namespace simparm;

    typedef int ADChannel;
    enum OutputAmp { ElectronMultiplication = 0, 
                     ConventionalAmplification = 1 };

    class _Config : public ConfigSet {
      public:
        /** The temperature the user WANTS to cool to. */
        ConfigEntryLong         targetTemperature;

        /** The selected output amplifier type. */
        ConfigEntryDataChoice< OutputAmp > outputAmp;
        /** The vertical scan speed that will be used by 
            *  acquisitions. */
        ConfigEntryDataChoice< int >       VS_Speed;
        /** The horizontal scan speed that will be used by 
            *  acquisitions. */
        ConfigEntryDataChoice< int >       HS_Speed;
        /** The EMCCD gain multiplier (if electron multiplying  
            *  amplification is used, which is true for now) */
        ConfigEntryUnsignedLong emccdGain;

        /** The actually used exposure time and kinetic cycle time. */
        ConfigEntryDouble       realExposureTime, cycleTime;

        _Config() throw();
        virtual ~_Config() throw(std::exception) {}

      protected:
        void registerNamedEntries() throw();
    };

    class Config : public _Config {
      public:
        Config() throw();
        Config(const Config& c) throw() : ConfigObject(c), _Config(c) 
            { registerNamedEntries(); }
    };
};
