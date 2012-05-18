#include "LinearAlignment.h"

#include <simparm/Structure.hh>
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterBuilder.h>
#include <boost/units/cmath.hpp>

#include <Eigen/Geometry>
#include <fstream>

namespace dStorm {
namespace output {

class LinearAlignment : public Filter
{
public:
    class Config;

private:
    Eigen::Affine2f transformation;

public:
    LinearAlignment( const Config&, std::auto_ptr< Output > );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);
    LinearAlignment* clone() const { return new LinearAlignment( *this ); }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class LinearAlignment::Config {
  public:
    simparm::FileEntry calibration_file;

    static std::string get_name() { return "LinearAlignment"; }
    static std::string get_description() { return "Apply linear alignment"; }
    static simparm::Object::UserLevel get_user_level() { return simparm::Object::Beginner; }

    Config() 
        : calibration_file("AlignmentFile", "Plane alignment file") {}

    bool determine_output_capabilities( Capabilities& cap ) 
        { return true; }
    void attach_ui( simparm::Node& at ) { calibration_file.attach_ui( at ); }
};

LinearAlignment::LinearAlignment( const Config& c, std::auto_ptr< Output > sub )
: Filter( sub )
{
    if ( c.calibration_file ) {
        std::ifstream input( c.calibration_file().c_str() );
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
                input >> transformation.matrix()( row, col );
    } else {
        transformation = Eigen::Affine2f::Identity();
    }
}

LinearAlignment::AdditionalData
LinearAlignment::announceStormSize(const Announcement& a) {
    return Filter::announceStormSize( a );
}

void LinearAlignment::receiveLocalizations(const EngineResult& upstream) {
    EngineResult r( upstream );

    EngineResult::iterator i, e = r.end();
    for ( i = r.begin(); i != e; ++i ) {
        i->position().head<2>() = from_value<si::length>( 
            transformation * value( i->position().head<2>() ) );
    }

    Filter::receiveLocalizations( r );
}

std::auto_ptr< OutputSource > make_linear_alignment()
{
    return std::auto_ptr< OutputSource >( 
        new dStorm::output::FilterBuilder<LinearAlignment::Config,LinearAlignment>() );
}

}
}

