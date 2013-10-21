#include "SigmaDiff3D.h"

#include <simparm/Object.h>
#include <simparm/FileEntry.h>

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/threed_info/Spline3D.h>
#include <dStorm/threed_info/look_up_sigma_diff.h>
#include <boost/units/cmath.hpp>

namespace dStorm {
namespace output {

class SigmaDiff3D : public Filter
{
public:
    class Config;

private:
    threed_info::Spline3D spline_x, spline_y;
    threed_info::SigmaDiffLookup lookup_table;

public:
    SigmaDiff3D( const Config&, std::auto_ptr< Output > );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);
};

class SigmaDiff3D::Config {
  public:
    simparm::FileEntry calibration_file;

    static std::string get_name() { return "SigmaDiff3D"; }
    static std::string get_description() { return "Look up 3D via sigma difference"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    Config() 
    : calibration_file("SigmaCalibrationFile", "Calibration file", "") 
    {
        calibration_file.setHelpID( "SigmaDiff3D_CalibrationFile" );
    }
    void attach_ui( simparm::NodeHandle at ) { calibration_file.attach_ui( at ); }

    bool determine_output_capabilities( Capabilities& cap ) 
        { return true; }
};

SigmaDiff3D::SigmaDiff3D( const Config& c, std::auto_ptr< Output > sub )
: Filter( sub ),
  spline_x( threed_info::SplineFactory(c.calibration_file(), Direction_X) ),
  spline_y( threed_info::SplineFactory(c.calibration_file(), Direction_Y) ),
  lookup_table( spline_x, spline_y, 1E-3 )
{
}

SigmaDiff3D::AdditionalData
SigmaDiff3D::announceStormSize(const Announcement& a) {
    if ( ! a.psf_width_x().is_given || ! a.psf_width_y().is_given )
        throw std::runtime_error("PSF width must be fitted and stored for sigma diff 3D");

    Announcement my_announcement(a);
    my_announcement.position_x().is_given = true;
    my_announcement.position_y().is_given = true;
    threed_info::ZRange z_range = spline_x.z_range() & spline_y.z_range();
    my_announcement.position_z().range().first = float(lower( z_range ) * 1E-6) * si::meter;
    my_announcement.position_z().range().second = float(upper( z_range ) * 1E-6) * si::meter;
    return Filter::announceStormSize( my_announcement );
}

void SigmaDiff3D::receiveLocalizations(const EngineResult& upstream) {
    EngineResult r( upstream );

    EngineResult::iterator i, e = r.end();
    for ( i = r.begin(); i != e; ++i ) {
        threed_info::Sigma sigmas[2];
        for (Direction j = Direction_First; j != Direction_2D; ++j)
          sigmas[j] = quantity<si::length>(i->psf_width(j)).value() * 1E6 / 2.35f;
        i->position().z() = lookup_table( sigmas[0], sigmas[1] ) * 1E-6 * si::meter;
    }

    r.erase( e, r.end() );
    Filter::receiveLocalizations( r );
}

std::auto_ptr< OutputSource > make_sigma_diff_3d()
{
    return std::auto_ptr< OutputSource >( 
        new dStorm::output::FilterBuilder<SigmaDiff3D::Config,SigmaDiff3D>() );
}

}
}

