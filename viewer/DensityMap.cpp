#include "viewer/fwd.h"
#include "viewer/DensityMapConfig.h"
#include "output/BasenameAdjustedFileEntry.h"
#include "output/Output.h"
#include "output/FileOutputBuilder.h"
#include "density_map/DensityMap.h"
#include "density_map/DummyListener.h"
#include <fstream>

namespace dStorm {
namespace viewer {

class DensityMapOutputConfig : public DensityMapConfig {
public:
    output::BasenameAdjustedFileEntry outputFile;
    DensityMapOutputConfig()
        : outputFile("ToFile", "Output file", "-density.txt") {}

    static std::string get_name() { return "DensityMap"; }
    static std::string get_description() { return "Density map"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    void attach_ui( simparm::NodeHandle at ) {
        outputFile.attach_ui(at);
        DensityMapConfig::attach_ui(at);
    }
};

class DensityMap : public output::Output {
    density_map::DummyListener<3> dummy_listener;
    density_map::DensityMap< density_map::DummyListener<3>, 3 > density;
    std::string filename;
public:
    DensityMap( const DensityMapOutputConfig& config ) 
        : density( &dummy_listener, config.binned_dimensions.make(), config.interpolator.make(), config.crop_border() ),
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
