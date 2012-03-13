#ifndef LOCPREC_SPOTMETER_H
#define LOCPREC_SPOTMETER_H
#include <map>
#include <iostream>
#include "Counter.h"
#include <dStorm/output/OutputBuilder.h>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>

namespace locprec {
    using namespace boost::units;

    class SpotMeter : public dStorm::output::OutputObject
    {
      private:
        typedef std::map<dStorm::engine::StormPixel,Counter> CountMap;
        CountMap countMap;
        int binSize;
        std::ostream &to;

        class _Config : public simparm::Object {
          protected:
            void registerNamedEntries() {
                push_back(targetFile);
                push_back(modulus);
            }

          public:
            simparm::FileEntry targetFile;
            simparm::Entry<unsigned long> modulus;

            _Config();
            bool can_work_with( dStorm::output::Capabilities )
                {return true;}
        };
        void store_results_( bool success ) {
            if ( success ) {
                for (CountMap::iterator i = countMap.begin(); 
                                        i != countMap.end(); i++)
                    to << i->first << " " << i->second << "\n";
            }
        }

      public:
        typedef simparm::VirtualStructure<_Config> Config;
        typedef dStorm::output::OutputBuilder<SpotMeter> Source;

        SpotMeter (Config& config) 
            : OutputObject("SpotMeter", ""), binSize(config.modulus()), 
              to(config.targetFile.get_output_stream()) {}
        SpotMeter *clone() const { return new SpotMeter(*this); }

        AdditionalData announceStormSize(const Announcement&)
            { return AdditionalData(); }
        RunRequirements announce_run(const RunAnnouncement&) {
            countMap.clear();
            return RunRequirements();
        }
        void receiveLocalizations(const EngineResult &er) 
 
        {
            for (EngineResult::const_iterator i = er.begin(); i != er.end(); ++i) {
                double realAmp = i->amplitude() / camera::ad_count;
                countMap[ int(realAmp / binSize) * binSize ]++;
            }
        }
    };

}

#endif
