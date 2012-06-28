#include "fwd.h"
#include "DensityMapConfig.h"
#include <dStorm/output/BasenameAdjustedFileEntry.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/outputs/BinnedLocalizations.h>
#include <fstream>

namespace dStorm {
namespace viewer {

class DensityMapOutputConfig : public DensityMapConfig {
public:
    output::BasenameAdjustedFileEntry outputFile;
    DensityMapOutputConfig()
        : outputFile("ToFile", "Output file", "-density.txt") {}

    static bool can_work_with(output::Capabilities) { return true; }

    static std::string get_name() { return "DensityMap"; }
    static std::string get_description() { return "Density map"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }
};

class DensityMap : public output::Output {
    outputs::BinnedLocalizations< outputs::DummyBinningListener<3>, 3 > density;
    std::string filename;
public:
    DensityMap( const DensityMapOutputConfig& config ) 
        : density( config.binned_dimensions.make(), config.interpolator.make(), config.crop_border() ),
          filename( config.outputFile() ) {}
    AdditionalData announceStormSize(const Announcement &a) {
        return density.announceStormSize( a );
    }
    RunRequirements announce_run(const RunAnnouncement& a) 
        { return density.announce_run( a ); }
    void receiveLocalizations(const EngineResult& er) 
        { density.receiveLocalizations(er); }
private:
    void store_results_( bool job_successful ) {
        density.store_results( job_successful );
        if ( job_successful ) {
            std::ofstream o( filename.c_str() );
            density.write_density_matrix(o);
        }
    }
    void attach_ui_( simparm::NodeHandle ) {}
};

std::auto_ptr<output::OutputSource> make_density_map_output_source() {
    return std::auto_ptr<output::OutputSource>( new output::FileOutputBuilder<DensityMapOutputConfig,DensityMap>() );
}
    
}
}
