#include "DriftRemover.h"

#include <simparm/Object.h>
#include <simparm/FileEntry.h>

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterBuilder.h>
#include <boost/units/cmath.hpp>

#include <Eigen/Geometry>
#include <fstream>

namespace dStorm {
namespace drift_remover {

class DriftRemover : public output::Filter
{
public:
    class Config;

private:
    std::map< frame_index, samplepos > correction;

public:
    DriftRemover( const Config&, std::auto_ptr< Output > );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class DriftRemover::Config {
  public:
    simparm::FileEntry drift_file;

    static std::string get_name() { return "DriftRemover"; }
    static std::string get_description() { return "Apply drift correction file"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    Config() 
        : drift_file("DriftFile", "Drift correction file") {}

    bool determine_output_capabilities( output::Capabilities& cap ) 
        { return true; }
    void attach_ui( simparm::NodeHandle at ) { drift_file.attach_ui( at ); }
};

DriftRemover::DriftRemover( const Config& c, std::auto_ptr< Output > sub )
: Filter( sub )
{
    if ( c.drift_file ) {
        std::ifstream input( c.drift_file().c_str() );
        while ( input ) {
            frame_index frame;
            samplepos pos;
            input >> quantity_cast<int&>(frame) 
                  >> quantity_cast<float&>( pos.x() )
                  >> quantity_cast<float&>( pos.y() )
                  >> quantity_cast<float&>( pos.z() );
            for (int i = 0; i < pos.rows(); ++i)
                pos[i] *= 1E-9f;
            if ( input )
                correction[ frame ] = pos;
        }
    }
}

DriftRemover::AdditionalData
DriftRemover::announceStormSize(const Announcement& a) {
    return Filter::announceStormSize( a );
}

void DriftRemover::receiveLocalizations(const EngineResult& upstream) {
    EngineResult r( upstream );

    EngineResult::iterator i, e = r.end();
    for ( i = r.begin(); i != e; ++i ) {
        i->position() -= correction[ i->frame_number() ];
    }

    Filter::receiveLocalizations( r );
}

std::auto_ptr< output::OutputSource > make()
{
    return std::auto_ptr< output::OutputSource >( 
        new dStorm::output::FilterBuilder<DriftRemover::Config,DriftRemover>() );
}

}
}


