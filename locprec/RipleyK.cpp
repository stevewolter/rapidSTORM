#include <Eigen/StdVector>
#include <simparm/BoostUnits.hh>
#include "DistanceHistogram.h"
#include "RipleyK.h"
#include <simparm/Entry_Impl.hh>
#include <simparm/FileEntry.hh>
#include <dStorm/units/nanolength.h>
#include <dStorm/output/binning/localization.h>
#include <dStorm/output/OutputBuilder.h>
#include <boost/ptr_container/ptr_array.hpp>
#include <dStorm/output/FileOutputBuilder.h>

namespace ripley_k {

using namespace dStorm::output::binning;
using boost::units::quantity;
namespace si = boost::units::si;

class Output : public dStorm::output::OutputObject {
    simparm::FileEntry filename;
    typedef Localization< dStorm::Localization::Fields::Position, ScaledByResolution, false > Scaler;
    boost::optional<distance_histogram::Histogram> histogram;
    boost::ptr_array< Scaler, 2 > scalers;
    class _Config;
    const quantity<si::length> bin_size;
    quantity<si::area> measured_area;
    long int localization_count;

  public:
    typedef simparm::Structure<_Config> Config;

    Output( const Config& );
    Output* clone() const;
    AdditionalData announceStormSize(const Announcement&);
    void propagate_signal(ProgressSignal);
    Result receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames)
    { 
        insert_filename_with_check( filename(), present_filenames ); 
    }
};

struct Output::_Config 
: public simparm::Object
{
    simparm::Entry< quantity<si::nanolength> > bin_size;
    dStorm::output::BasenameAdjustedFileEntry outputFile;

  public:
    _Config() 
        : simparm::Object("RipleyK", "Compute Ripley's K function"),
          bin_size("BinSize", "Bin size", 5 * si::nanometre),
          outputFile("ToFile", "Output file", "-ripley-k.txt") {}
    void registerNamedEntries() { push_back( bin_size ); push_back( outputFile ); }
    bool can_work_with( dStorm::output::Capabilities ) { return true; }
};

Output* Output::clone() const { return new Output(*this); }

Output::Output( const Config& config ) 
: OutputObject("RipleyK", "Ripley K function computation"),
  filename( config.outputFile ),
  bin_size( config.bin_size() )
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

Output::Result Output::receiveLocalizations(const EngineResult& e) {
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i ) {
        boost::array< float, 2 > line;
        for (int j = 0; j < 2; ++j)
            line[j] = scalers[j].bin_point( *i );
        histogram->push_back( line );
    }
    localization_count += e.size();
    return KeepRunning;
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

void Output::propagate_signal(ProgressSignal e) {
    if ( e == Engine_is_restarted && histogram.is_initialized() ) {
        histogram->clear();
    } else if ( e == Engine_run_succeeded && histogram.is_initialized() ) {
        std::ostream& output = filename.get_output_stream();

        typedef  boost::units::power_typeof_helper< si::area, boost::units::static_rational<-1> >::type per_area;
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
            output << bin_center.value() << "\t" 
                      << ring_area.value() << "\t"
                      << (h.counts[i] * 2) << "\t"
                      << (h.counts[i] * 2.0 / localization_count / (ring_area * average_count_density)).value() << " "
                      << ripley_k.value() << " "
                      << ripley_l.value() << " "
                      << (ripley_l - bin_outer_edge).value() << " "
                      << "\n";
        }
    }
}

}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<ripley_k::Output>()
{
    return std::auto_ptr<OutputSource>( new dStorm::output::FileOutputBuilder<ripley_k::Output>() );
}

}
}
