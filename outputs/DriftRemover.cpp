#include "DriftRemover.h"

#include <simparm/Object.h>
#include <simparm/FileEntry.h>

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterBuilder.h>
#include <boost/units/cmath.hpp>
#include <boost/range/algorithm/lower_bound.hpp>

#include <Eigen/Geometry>
#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <fstream>
#include <iostream>

namespace br = boost::range;
namespace bu = boost::units;

namespace Eigen {
namespace internal {

template <typename Unit, typename Scalar>
struct scalar_product_traits< bu::quantity<Unit,Scalar>, Scalar > {
    typedef bu::quantity<Unit,Scalar> ReturnType;
};

}
}

namespace dStorm {
namespace drift_remover {

struct ControlPoint {
    frame_index time;
    samplepos correction;

    bool operator< ( const ControlPoint& o ) { return time < o.time; }
    bool operator< ( frame_index t ) { return time < t; }
};

std::istream& operator>>( std::istream& i, ControlPoint& rv ) {
    float t, p[3];
    i >> t >> p[0] >> p[1] >> p[2];
    rv.time = int(round(t)) * camera::frame;
    for (int d = 0; d < 3; ++d)
        rv.correction[d] = p[d] * 1E-9f * si::metre;
    return i;
}

class Config;

class DriftRemover : public output::Filter
{
private:
    typedef std::vector< ControlPoint > Corrections;
    Corrections correction;
    frame_index range;
    bu::quantity<bu::camera::time,float> sigma;

public:
    DriftRemover( const Config&, std::auto_ptr< Output > );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class Config {
  public:
    simparm::FileEntry drift_file;
    simparm::Entry< quantity<camera::time,float> > smoothing;

    static std::string get_name() { return "DriftRemover"; }
    static std::string get_description() { return "Apply drift correction file"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    Config() 
        : drift_file("DriftFile", "Drift correction file"),
          smoothing("Smoothing", "Smoothing width", 5.0f * bu::camera::frame) {}

    bool determine_output_capabilities( output::Capabilities& cap ) 
        { return true; }
    void attach_ui( simparm::NodeHandle at ) { drift_file.attach_ui( at ); smoothing.attach_ui(at); }
};

DriftRemover::DriftRemover( const Config& c, std::auto_ptr< Output > sub )
: Filter( sub ),
  range( round( c.smoothing() * 4.0f ) ),
  sigma( c.smoothing() )
{
    if ( c.drift_file ) {
        std::ifstream input( c.drift_file().c_str() );
        while ( input ) {
            while ( isspace( input.peek() ) ) input.get();
            if ( input.peek() == '#' ) { std::string comment; std::getline( input, comment ); }
            ControlPoint p;
            input >> p;
            if ( input ) {
                if ( ! (correction.empty() || correction.back() < p) ) {
                    throw std::runtime_error("Drift correction file is not monotonously growing");
                }
                correction.push_back( p );
            }
        }
        if ( correction.empty() )
            throw std::runtime_error("Drift correction file is empty");
    }
}

DriftRemover::AdditionalData
DriftRemover::announceStormSize(const Announcement& a) {
    return Filter::announceStormSize( a );
}

void DriftRemover::receiveLocalizations(const EngineResult& upstream) {
    EngineResult r( upstream );

    frame_index t = r.frame_number();
    Corrections::const_iterator closest = br::lower_bound( correction, t ), begin = closest, end = closest;
    while ( begin != correction.begin() && (begin-1)->time >= t - range ) --begin;
    while ( end != correction.end() && end->time <= t + range ) ++end;

    samplepos final_correction;
    if ( begin == end || sigma < 1E-2 * camera::frame ) {
        if ( closest == correction.end() )
            final_correction = (closest-1)->correction;
        else if ( closest == correction.begin() )
            final_correction = closest->correction;
        else if ( abs(closest->time - t) < abs( (closest-1)->time - t ) )
            final_correction = closest->correction;
        else 
            final_correction = (closest-1)->correction;
    } else {
        samplepos accumulator = samplepos::Constant( 0.0f * si::metre );
        float total_weight = 0;

        for ( Corrections::const_iterator i = begin; i != end; ++i ) {
            float weight = exp( - pow<2>( (i->time - t) / sigma ) );
            accumulator += (i->correction.array() * weight).matrix();
            total_weight += weight;
        }
        final_correction = accumulator.array() * (1.0f / total_weight);
    }

    EngineResult::iterator i, e = r.end();
    for ( i = r.begin(); i != e; ++i ) {
        i->position() -= final_correction;
    }

    Filter::receiveLocalizations( r );
}

std::auto_ptr< output::OutputSource > make()
{
    return std::auto_ptr< output::OutputSource >( 
        new dStorm::output::FilterBuilder<Config,DriftRemover>() );
}

}
}


