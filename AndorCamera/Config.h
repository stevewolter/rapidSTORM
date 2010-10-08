#ifndef ANDORCAMERA_CONFIG_H
#define ANDORCAMERA_CONFIG_H

#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <simparm/Set.hh>
#include <simparm/Structure.hh>
#include <dStorm/UnitEntries/TemperatureEntry.h>
#include <dStorm/UnitEntries/TimeEntry.h>

namespace AndorCamera {
    typedef int ADChannel;
    enum OutputAmp { ElectronMultiplication = 0, 
                     ConventionalAmplification = 1 };
    std::ostream& operator<<(std::ostream&, OutputAmp);

    enum ReadoutMode { Full_Vertical_Binning = 0, Multi_Track = 1,
                Random_Track = 2, Single_Track = 3, Image = 4 };
    std::ostream& operator<<(std::ostream&, ReadoutMode);

    enum AcquisitionMode {
        Single_Scan = 1, Accumulate = 2, Kinetics = 3,
        Fast_Kinetics = 4, Run_till_abort = 5, 
        Time_Delayed_Integration = 9
    };
    std::ostream& operator<<(std::ostream&, AcquisitionMode);

    class _Config : public simparm::Set {
      public:
        /** The temperature the user WANTS to cool to. */
        dStorm::IntCelsiusEntry targetTemperature;

        /** The selected output amplifier type. */
        simparm::DataChoiceEntry< OutputAmp > outputAmp;
        /** The vertical scan speed that will be used by 
            *  acquisitions. */
        simparm::DataChoiceEntry< int >       VS_Speed;
        /** The horizontal scan speed that will be used by 
            *  acquisitions. */
        simparm::DataChoiceEntry< int >       HS_Speed;
        /** The EMCCD gain multiplier (if electron multiplying  
            *  amplification is used, which is true for now) */
        simparm::NumericEntry<int>            emccdGain;

        /** The actually used exposure time and kinetic cycle time. */
        dStorm::FloatTimeEntry                realExposureTime, cycleTime;

        _Config();

      protected:
        void registerNamedEntries();
    };

    class Config : public simparm::Structure<_Config> {};
}

#endif
