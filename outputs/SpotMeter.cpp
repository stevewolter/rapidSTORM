#include "SpotMeter.h"
#include <dStorm/output/FileOutputBuilder.h>
#include <simparm/FileEntry.h>
#include <simparm/Entry.h>
#include <map>
#include <fstream>

namespace dStorm { 
namespace outputs {

using namespace boost::units;

class SpotMeter : public dStorm::output::Output
{
private:
    typedef std::map<dStorm::engine::StormPixel,int> CountMap;
    CountMap countMap;
    int binSize;
    std::string output_file;

    void store_results_( bool success ) {
        if ( success ) {
            std::ofstream to( output_file.c_str() );
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
        void attach_ui( simparm::NodeHandle at ) {
            outputFile.attach_ui(at);
            modulus.attach_ui(at);
        }
        static std::string get_name() { return "SpotMeter"; }
        static std::string get_description() { return "Histogram localization amplitudes"; }
        static simparm::UserLevel get_user_level() { return simparm::Intermediate; }
    };
    SpotMeter (Config& config) 
        : binSize(config.modulus()), 
            output_file(config.outputFile()) {}

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
}
