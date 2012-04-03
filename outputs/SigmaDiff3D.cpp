#include "SigmaDiff3D.h"

#include <simparm/Structure.hh>
#include <simparm/Object.hh>
#include <simparm/FileEntry.hh>

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/threed_info/Spline.h>

namespace dStorm {
namespace output {

class SigmaDiff3D : public Filter
{
public:
    class _Config;
    typedef simparm::Structure<_Config> Config;

private:
    threed_info::Spline spline;

public:
    SigmaDiff3D( const Config&, std::auto_ptr< Output > );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);
    SigmaDiff3D* clone() const { return new SigmaDiff3D( *this ); }
};

class SigmaDiff3D::_Config : public simparm::Object {
  protected:
    void registerNamedEntries() { push_back( calibration_file ); }
  public:
    simparm::FileEntry calibration_file;

    _Config() 
        : Object("SigmaDiff3D", "Look up 3D via sigma difference"),
          calibration_file("SigmaCalibrationFile", "Calibration file") {}

    bool determine_output_capabilities( Capabilities& cap ) 
        { return true; }
};

SigmaDiff3D::SigmaDiff3D( const Config& c, std::auto_ptr< Output > sub )
: Filter( sub ),
  spline( threed_info::SplineFactory(c.calibration_file()) )
{
}

SigmaDiff3D::AdditionalData
SigmaDiff3D::announceStormSize(const Announcement& a) {
    if ( ! a.covariance_matrix().is_given(0,0) || ! a.covariance_matrix().is_given(1,1) )
        throw std::runtime_error("PSF width must be fitted and stored for sigma diff 3D");

    Announcement my_announcement(a);
    my_announcement.position().is_given[2] = true;
    my_announcement.position().range().z().first = spline.get_range().first;
    my_announcement.position().range().z().second = spline.get_range().second;
    return Filter::announceStormSize( my_announcement );
}

void SigmaDiff3D::receiveLocalizations(const EngineResult& upstream) {
    EngineResult r( upstream );

    EngineResult::iterator i, e = r.end();
    for ( i = r.begin(); i != e; ) {
        threed_info::Spline::ZPosition pos = 
            spline.look_up_sigma_diff( *i, 1E-9 * si::meter );
        if ( pos ) {
            i->position().z() = *pos;
            ++i;
        } else {
            --e;
            *i = *e;
        }
    }

    r.erase( e, r.end() );
    Filter::receiveLocalizations( r );
}

std::auto_ptr< OutputSource > make_sigma_diff_3d()
{
    return std::auto_ptr< OutputSource >( 
        new dStorm::output::FilterBuilder<SigmaDiff3D>() );
}

}
}

