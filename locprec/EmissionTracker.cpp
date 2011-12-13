#include <simparm/BoostUnits.hh>
#include "EmissionTracker.h"
#include <simparm/Entry_Impl.hh>
#include <algorithm>
#include <numeric>
#include <boost/units/Eigen/Array>
#include <dStorm/helpers/back_inserter.h>
#include <dStorm/output/binning/localization.h>
#include <dStorm/image/iterator.h>
#include <boost/units/Eigen/Array>

using namespace dStorm;
using namespace std;
using namespace boost::units::camera;

namespace locprec {
namespace emission_tracker {

int traceNumber = 0;
static int number_of_emission_nodes = 0;

template <typename T>
T sq(const T& a) { return a*a; }

#if 0
double distSq(const Localization& e, const Localization &loc) {
    return ( sq(e.x()/pixel - loc.x()/pixel)
           + sq(e.y()/pixel - loc.y()/pixel) );
}
#endif

struct Output::TrackingInformation {
    int imageNumber;
    std::set< TracedObject* > emissions;
    Eigen::Vector2d displacement;

    TrackingInformation() { imageNumber = -5; }

    void prepare(int imageNumber ) {
        this->imageNumber = imageNumber;
        assert( emissions.empty() );
    }

    static bool all_nodes_accounted_for(
        const std::vector<TrackingInformation*>& v,
        int garbage_bin_size
    ) {
        int acc = garbage_bin_size;
        for (unsigned int i = 0; i < v.size(); i++)
            acc += v[i]->emissions.size();
        return ( acc == number_of_emission_nodes );
    }
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

Output::TracedObject::TracedObject(const Output& papa)
: KalmanTrace<2>(papa.kalman_info)
{
}
Output::TracedObject::~TracedObject() {
}

Output::_Config::_Config() 
: simparm::Object("EmissionTracker", "Track emissions"),
  allowBlinking("AllowBlinking", "Allow fluorophores to skip n frames"),
  expectedDeviation("ExpectedDeviation", "SD of expected distance between tracked localizations", 
    20 * boost::units::si::nanometre),
  diffusion("DiffusionConstant", "Diffusion constant"),
  mobility("Mobility", "Mobility constant"),
  distance_threshold("DistanceThreshold", "Distance threshold", 2)
{
    allowBlinking.helpID = "EmissionTracker.Allow_Blinking";
    expectedDeviation.helpID = "EmissionTracker.Expected_Deviation";
    diffusion.helpID = "EmissionTracker.Diffusion_Constant";
    mobility.helpID = "EmissionTracker.Mobility_Constant";
}

void Output::_Config::registerNamedEntries()

{
    push_back( distance_threshold );
    push_back( allowBlinking );
    push_back( expectedDeviation );
    push_back( diffusion );
    push_back( mobility );
    push_back( reducer );
}

Output::Output( 
    const Config &config,
    std::auto_ptr<dStorm::output::Output> output )
: OutputObject("EmissionTracker", "Emission tracking status"),
  track_modulo( config.allowBlinking()+2 ), 
  reducer( config.reducer.make_trace_reducer() ), 
  target(output),
  maxDist( config.distance_threshold() )
{
    for (int i = 0; i < 2; ++i) {
        binners.replace(i, new dStorm::output::binning::Localization<0,dStorm::output::binning::ScaledToInterval,true>(50,i,0) );
        kalman_info.set_measurement_covariance(i, pow<2>( quantity<si::length>(config.expectedDeviation()) ));
        kalman_info.set_diffusion(i, config.diffusion() * 2.0);
        kalman_info.set_mobility(i, config.mobility());
    }
    push_back(target->getNode());
}

Output::~Output() {
}

output::Output::AdditionalData
Output::announceStormSize(const Announcement &a) 
{
    Announcement my_announcement(a);
    static_cast< dStorm::input::Traits<Localization>& >(my_announcement) 
        = dStorm::input::Traits<Localization>();
    my_announcement.in_sequence = true;
    my_announcement.position() = a.position();
    my_announcement.amplitude() = a.amplitude();
    my_announcement.image_number() = a.image_number();
    my_announcement.source_traits.push_back( 
        boost::shared_ptr< dStorm::input::Traits<Localization> >( new dStorm::input::Traits<Localization>(a) ) );

    dStorm::ImageTypes<2>::Size sizes;
    for (int i = 0; i < 2; ++i) {
        binners[i].announce(a);
        binner_starts[i] = binners[i].get_minmax().first;
        sizes[i] = int(ceil(binners[i].get_size())) * boost::units::camera::pixel;
    }

    positional = Positional(sizes);
    while ( int(tracking.size()) < track_modulo )
        tracking.push_back( new TrackingInformation() );
    AdditionalData childData = target->announceStormSize(my_announcement);
    Output::check_additional_data_with_provided
        ( simparm::Object::name, AdditionalData().set_cluster_sources(),
            childData );
    return AdditionalData();
}

struct Output::lowest_mahalanobis_distance
: public std::binary_function< TracedObject*, TracedObject*, TracedObject* > {
    const Localization& to;
    const std::set<TracedObject*>& excluded;
    const double max_sq_distance;
    lowest_mahalanobis_distance(
        const Localization& to, const std::set<TracedObject*>& excluded,
        double threshold) : to(to), excluded(excluded), max_sq_distance(threshold) {}

    TracedObject* operator()( TracedObject* a, TracedObject* b ) {
        if ( excluded.find(a) != excluded.end() ) a = NULL;
        if ( excluded.find(b) != excluded.end() ) b = NULL;

        double dist_a = (a == NULL) ? (max_sq_distance+1) : a->sq_distance_in_sigmas( to ), 
               dist_b = (b == NULL) ? (max_sq_distance+1) : b->sq_distance_in_sigmas(to);

        if ( dist_a < dist_b && dist_a < max_sq_distance )
        {
            return a;
        } else if ( dist_b < dist_a && dist_b < max_sq_distance ) {
            return b;
        } else
            return NULL;
    }
};

Output::TracedObject*
Output::search_closest_trace(
    const Localization &loc,
    const std::set<TracedObject*>& excluded
    )
{
    int range[2][2];
    for (int i = 0; i < 2; ++i) {
        range[i][0] = int(floor(binners[i].bin_point(loc)-maxDist));
        range[i][1] = int(ceil(binners[i].bin_point(loc)+maxDist));
        for (int j = 0; j < 2; ++j) {
            range[i][j] = std::max(0, std::min( range[i][j], positional.sizes()[i].value()-1 ) );
        }
    }

    TracedObject* best = NULL;
    for (int x = range[0][0]; x < range[0][1]; ++x)
        for (int y = range[1][0]; y < range[1][1]; ++y)
            best = std::accumulate( 
                positional(x,y).begin(), positional(x,y).end(), 
                best, lowest_mahalanobis_distance(loc, excluded, maxDist * maxDist) );
    
    return best;
}

void
Output::update_positional( TracedObject& object ) 
{
    dStorm::Localization estimate;
    estimate.position().head<2>() = 
        boost::units::from_value< boost::units::si::length >(object.getPositionEstimate().cast<float>());
    Eigen::Array2i new_pos;
    new_pos.x() = round(binners[0].bin_point(estimate));
    new_pos.y() = round(binners[1].bin_point(estimate));

    new_pos = new_pos.cwiseMax( Eigen::Array2i::Zero() )
                     .cwiseMin( boost::units::value( positional.sizes() ).array() - 1 );

    if ( ! object.cache_position.is_initialized() ) {
        positional( new_pos.x(), new_pos.y() ).insert( &object );
    } else if ( (object.cache_position->array() != new_pos).any() ) {
        positional( object.cache_position->x(), object.cache_position->y() ).erase( &object );
        positional( new_pos.x(), new_pos.y() ).insert( &object );
    }

    object.cache_position = new_pos;
}

output::Output::Result
Output::receiveLocalizations(const EngineResult &er)
{
    int imNum = er.forImage / frame;
    TrackingInformation &current = tracking[imNum % track_modulo];

    current.prepare( imNum );

    for (EngineResult::const_iterator loc = er.begin(); loc != er.end(); ++loc) 
    {
        TracedObject* trace = search_closest_trace( *loc, current.emissions );
        if ( trace == NULL ) {
            std::auto_ptr<TracedObject> new_trace( new TracedObject(*this) );
            trace = new_trace.get();
            traced_objects.insert( new_trace );
        }

        trace->add( *loc );
        update_positional( *trace );
        for (int i= 1; i < track_modulo && i <= imNum; ++i) {
            tracking[(imNum-i) % track_modulo].emissions.erase( trace );
        }
        current.emissions.insert( trace );
    }

    if ( imNum >= track_modulo-1)
        finalizeImage(imNum - track_modulo + 1);

    return KeepRunning;
}

void Output::finalizeImage(int imNum) {
    TrackingInformation &data = tracking[imNum % track_modulo];

    EngineResult er;
    er.forImage = imNum * frame;

    for ( std::set<TracedObject*>::const_iterator i = data.emissions.begin(), end = data.emissions.end();
          i != end; i++ ) 
    {
        TracedObject& o = **i;
        reducer->reduce_trace_to_localization( 
            o.begin(), o.end(),
            boost::back_inserter(static_cast<std::vector<dStorm::Localization>&>(er)), 
            dStorm::samplepos::Constant(0*si::meter) );
        positional( o.cache_position->x(), o.cache_position->y() ).erase( &o );
        traced_objects.erase( o );
    }

    data.emissions.clear();
    target->receiveLocalizations(er);
}

void Output::propagate_signal(ProgressSignal s) {
    if ( s == Engine_run_succeeded ) {
        /* TODO: Emit last few images that are in the ring buffer but not finalized */
    }
    if ( target.get() != NULL )
        target->propagate_signal(s);
}

}
}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<locprec::emission_tracker::Output>()
{
    return std::auto_ptr<OutputSource>( new locprec::emission_tracker::Output::Source() );
}

}
}
