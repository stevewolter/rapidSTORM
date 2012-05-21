#include "SpotMeter.h"
#include <dStorm/output/FileOutputBuilder.h>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <map>
#include <iostream>

namespace locprec {
    using namespace boost::units;

class SpotMeter : public dStorm::output::Output
{
private:
    typedef std::map<dStorm::engine::StormPixel,int> CountMap;
    CountMap countMap;
    int binSize;
    std::ostream &to;

    void store_results_( bool success ) {
        if ( success ) {
            for (CountMap::iterator i = countMap.begin(); 
                                    i != countMap.end(); i++)
                to << i->first << " " << i->second << "\n";
        }
    }

public:
    struct Config {
        simparm::FileEntry outputFile;
        simparm::Entry<unsigned long> modulus;

        Config();
        bool can_work_with( dStorm::output::Capabilities ) {return true;}
        void attach_ui( simparm::Node& at ) {
            outputFile.attach_ui(at);
            modulus.attach_ui(at);
        }
        static std::string get_name() { return "SpotMeter"; }
        static std::string get_description() { return "Histogram localization amplitudes"; }
    };
    SpotMeter (Config& config) 
        : binSize(config.modulus()), 
            to(config.outputFile.get_output_stream()) {}
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
            int bin = int(realAmp / binSize) * binSize;
            CountMap::iterator place = countMap.insert( std::make_pair( bin, 0 ) ).first;
            ++ place->second;
        }
    }
};

SpotMeter::Config::Config()
: outputFile("OutputFile", "Histogram target file"),
  modulus("BinSize", "Size for histogram bins") 
{ 
    modulus.min = (1); 
}

std::auto_ptr< dStorm::output::OutputSource > make_spot_meter_source() {
    return std::auto_ptr< dStorm::output::OutputSource >( 
        new dStorm::output::OutputBuilder< SpotMeter::Config, SpotMeter >() );
}

}
