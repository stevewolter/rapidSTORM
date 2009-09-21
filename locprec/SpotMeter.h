#ifndef LOCPREC_SPOTMETER_H
#define LOCPREC_SPOTMETER_H
#include <map>
#include <iostream>
#include "Counter.h"
#include <dStorm/OutputBuilder.h>
#include <simparm/FileEntry.hh>
#include <simparm/NumericEntry.hh>

namespace locprec {
    class SpotMeter : public simparm::Object, public dStorm::Output
    {
      private:
        typedef std::map<dStorm::StormPixel,Counter> CountMap;
        CountMap countMap;
        int binSize;
        ost::Mutex mutex;
        std::ostream &to;

        class _Config : public simparm::Object {
          protected:
            void registerNamedEntries() {
                push_back(targetFile);
                push_back(modulus);
            }

          public:
            simparm::FileEntry targetFile;
            simparm::UnsignedLongEntry modulus;

            _Config();
        };

      public:
        typedef simparm::VirtualStructure<_Config> Config;
        typedef dStorm::OutputBuilder<SpotMeter> Source;

        SpotMeter (Config& config) 
            : Object("SpotMeter", ""), binSize(config.modulus()), 
              to(config.targetFile.get_output_stream()) {}
        SpotMeter *clone() const { return new SpotMeter(*this); }

        AdditionalData announceStormSize(const Announcement&)
 { return NoData; }
        Result receiveLocalizations(const EngineResult &er) 
 
        {
            ost::MutexLock lock(mutex);
            for (int f = 0; f < er.number; f++) {
                double realAmp = er.first[f].getStrength();
                countMap[ int(realAmp / binSize) * binSize ]++;
            }
            return KeepRunning;
        }
        void propagate_signal(ProgressSignal s) {
            ost::MutexLock lock(mutex);
            if ( s == Engine_is_restarted )
                countMap.clear();
            else if ( s == Engine_run_succeeded )
                for (CountMap::iterator i = countMap.begin(); 
                                        i != countMap.end(); i++)
                    to << i->first << " " << i->second << "\n";
        }
    };

}

#endif
