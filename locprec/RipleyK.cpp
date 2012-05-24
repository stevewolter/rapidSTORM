#include <Eigen/StdVector>
#include <simparm/BoostUnits.h>
#include "DistanceHistogram.h"
#include "RipleyK.h"
#include <simparm/Entry_Impl.h>
#include <simparm/FileEntry.h>
#include <dStorm/units/nanolength.h>
#include <dStorm/output/binning/localization.h>
#include <dStorm/output/OutputBuilder.h>
#include <boost/ptr_container/ptr_array.hpp>
#include <dStorm/output/FileOutputBuilder.h>
#include <dStorm/units/microlength.h>

namespace ripley_k {

using namespace dStorm::output::binning;
using boost::units::quantity;
namespace si = boost::units::si;

struct Config 
{
    simparm::Entry< quantity<si::nanolength> > bin_size;
    dStorm::output::BasenameAdjustedFileEntry outputFile;

  public:
    Config() 
        : bin_size("BinSize", "Bin size", 5 * si::nanometre),
          outputFile("ToFile", "Output file", "-ripley-k.txt") {}
    void attach_ui( simparm::NodeHandle at ) { bin_size.attach_ui(at); outputFile.attach_ui( at ); }
    bool can_work_with( dStorm::output::Capabilities ) { return true; }
    static std::string get_name() { return "RipleyK"; }
    static std::string get_description() { return "Compute Ripley's K function"; }
};


class Output : public dStorm::output::Output {
    simparm::FileEntry filename;
    typedef Localization< dStorm::Localization::Fields::Position, ScaledByResolution > Scaler;
    boost::optional<distance_histogram::Histogram> histogram;
    boost::ptr_array< Scaler, 2 > scalers;
    const quantity<si::length> bin_size;
    quantity<si::area> measured_area;
    long int localization_count;
    void store_results_( bool success );

  public:
    Output( const Config& );
    AdditionalData announceStormSize(const Announcement&);
    void receiveLocalizations(const EngineResult&);
    RunRequirements announce_run(const RunAnnouncement&) { 
        histogram->clear();
        localization_count = 0;
        return RunRequirements(); 
    }

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
    { 
        insert_filename_with_check( filename(), present_filenames ); 
    }
};

Output::Output( const Config& config ) 
: filename( config.outputFile ),
  bin_size( config.bin_size() ),
  localization_count( 0 )
{
    Scaler::value v( config.bin_size() );
    for (int i = 0; i < 2; ++i )
        scalers.replace(i, new Scaler(v,i,0) );
}

Output::AdditionalData Output::announceStormSize(const Announcement& a) {
    for (int i = 0; i < 2; ++i )
        scalers[i].announce(a);
    boost::array< float, 2 > range;
    for (int i = 0; i < 2; ++i) 
        range[i] = scalers[i].get_size();
    histogram = distance_histogram::Histogram( range );
    measured_area = 
        ( *a.position().range().x().second - *a.position().range().x().first ) *
        ( *a.position().range().y().second - *a.position().range().y().first );
    return Output::AdditionalData();
}

void Output::receiveLocalizations(const EngineResult& e) {
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i ) {
        boost::array< float, 2 > line;
        bool is_good = true;
        for (int j = 0; j < 2; ++j) {
            boost::optional<float> v = scalers[j].bin_point( *i );
            if ( ! v ) { is_good = false; continue; }
            line[j] = *v;
        }
        if ( is_good )
            histogram->push_back( line );
    }
    localization_count += e.size();
}

static quantity<si::area> area_of_ring( 
    quantity<si::length> middle_radius, quantity<si::length> thickness
) {
    quantity<si::length> outer_radius = middle_radius + 0.5 * thickness;
    if ( outer_radius < thickness ) 
        return M_PI * outer_radius * outer_radius;
    else
        return M_PI * thickness * (2.0 * outer_radius - thickness);
}

void Output::store_results_( bool success ) {
    if ( histogram.is_initialized() ) {
        std::ostream& output = filename.get_output_stream();

        typedef  boost::units::power_typeof_helper< si::area, boost::units::static_rational<-1> >::type per_area;
        typedef  boost::units::power_typeof_helper< si::microlength, boost::units::static_rational<2> >::type microarea;
        quantity<per_area>
            average_count_density = (localization_count * 1.0) / measured_area;
        distance_histogram::Histogram& h = *histogram;
        h.compute();
        unsigned long accum = 0;
        for (unsigned int i = 0; i < h.counts.size(); ++i) {
            quantity<si::length> 
                bin_center = quantity<si::length>::from_value( scalers[0].reverse_mapping( i ) ),
                bin_outer_edge = quantity<si::length>::from_value( scalers[0].reverse_mapping( i+0.5 ) );
            quantity<si::area> ring_area = area_of_ring( bin_center, bin_size );
            accum += h.counts[i] * 2;

            quantity<si::area> ripley_k = (accum * 1.0 / (average_count_density * (1.0 * localization_count)));
            quantity<si::length> ripley_l = sqrt(ripley_k / M_PI);
            output << quantity<si::microlength>(bin_center).value() << "\t" 
                      << quantity<microarea>(ring_area).value() << "\t"
                      << (h.counts[i] * 2) << "\t"
                      << quantity<si::dimensionless>(h.counts[i] * 2.0 / localization_count / (ring_area * average_count_density)).value() << "\t"
                      << quantity<microarea>(ripley_k).value() << "\t"
                      << quantity<si::microlength>(ripley_l).value() << "\t"
                      << quantity<si::microlength>(ripley_l - bin_outer_edge).value() 
                      << "\n";
        }
    }
}

std::auto_ptr<dStorm::output::OutputSource> make_output_source()
{
    return std::auto_ptr<dStorm::output::OutputSource>( new dStorm::output::FileOutputBuilder<Config,Output>() );
}

}
