#include "kalman_filter/EmissionTracker.h"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <vector>

#include <boost/optional/optional.hpp>
#include <boost/ptr_container/ptr_array.hpp>
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/units/Eigen/Array>
#include <boost/utility/in_place_factory.hpp>
#include <Eigen/Core>

#include "binning/binning.h"
#include "binning/binning.hpp"
#include "binning/localization_impl.h"
#include "helpers/back_inserter.h"
#include "helpers/make_unique.hpp"
#include "image/iterator.h"
#include "kalman_filter/KalmanTrace.h"
#include "output/FilterBuilder.h"
#include "output/Filter.h"
#include "simparm/BoostUnits.h"
#include "simparm/Entry.h"

using namespace std;
using namespace boost::units::camera;

namespace dStorm {
namespace kalman_filter {
namespace emission_tracker {
namespace {

template <typename Type>
struct address_is_less : public std::binary_function<Type,Type,bool> {
    bool operator()( const Type& a, const Type& b ) const { return &a < &b; };
};

class TracedObject {
  public:
    boost::optional<Eigen::Vector2i> cache_position;

    TracedObject(const KalmanMetaInfo& kalman_info, int molecule_id);

    void add( const dStorm::Localization& l) {
        trace.add(make_observation(l));
        result.push_back(l);
        result.back().molecule() = result.group;
    }

    float sq_distance_in_sigmas(const Localization& position) {
        return trace.sq_distance_in_sigmas(make_observation(position));
    }

    Localization position_estimate() {
        Localization result;
        Eigen::Vector2f position_estimate = trace.getPositionEstimate().cast<float>();
        result.position_x() = position_estimate.x() * si::meter;
        result.position_y() = position_estimate.y() * si::meter;
        return result;
    }

    const output::LocalizedImage& engine_result() const { return result; }

  private:
    KalmanTrace::Observation make_observation(const Localization& l) {
        Eigen::Vector2d position;
        Eigen::Matrix2d covariance = Eigen::Matrix2d::Zero();
        for (int i = 0; i < 2; ++i) {
            position[i] = l.position(i) / si::metre;
            covariance(i,i) = pow<2>(l.position_uncertainty(i) / si::metre);
        }
        int time = l.frame_number() / camera::frame;
        return {position, covariance, time};
    }

    output::LocalizedImage result;
    KalmanTrace trace;
};

class Output 
: public dStorm::output::Filter
{
  public:
    typedef emission_tracker::Config Config;
private:
    class lowest_mahalanobis_distance;

    std::auto_ptr<binning::Scaled> binners[2];
    float binner_starts[2];
    typedef dStorm::Image< std::set<TracedObject*>, 2 > Positional;
    Positional positional;

    const int track_modulo;
    frame_index last_seen_frame;
    int molecule_id;

    boost::ptr_set< TracedObject, address_is_less<TracedObject> > traced_objects;

    struct TrackingInformation;
    boost::ptr_vector<TrackingInformation> tracking;

    KalmanMetaInfo kalman_info;

    TracedObject* search_closest_trace(
        const dStorm::Localization &loc, 
        const std::set<TracedObject*>& excluded);
    void update_positional( TracedObject& object );
    void finalizeImage(int i);

    const double maxDist;

public:
    Output( const Config& config,
                        std::auto_ptr<dStorm::output::Output> output );
    ~Output() OVERRIDE;
    Output( const Output& ) = delete;
    Output& operator=( const Output& ) = delete;

    void announceStormSize(const Announcement &) OVERRIDE;
    void receiveLocalizations(const EngineResult &) OVERRIDE;
    void store_results_( bool success ) OVERRIDE;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

struct Output::TrackingInformation {
    int imageNumber;
    std::set< TracedObject* > emissions;
    Eigen::Vector2d displacement;

    TrackingInformation() { imageNumber = -5; }

    void prepare(int imageNumber ) {
        this->imageNumber = imageNumber;
        assert( emissions.empty() );
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

TracedObject::TracedObject(const KalmanMetaInfo& kalman_info, int molecule_id)
: trace(kalman_info) {
    result.group = molecule_id;
}

Output::Output( 
    const Config &config,
    std::auto_ptr<dStorm::output::Output> output )
: Filter(output),
  track_modulo( config.allowBlinking().value()+2 ), 
  last_seen_frame( 0 * camera::frame ),
  molecule_id(0),
  maxDist( config.distance_threshold() )
{
    binners[0] = binning::make_BinningAdapter<binning::Scaled>(binning::Localization<localization::PositionX, binning::ScaledToInterval>(100));
    binners[1] = binning::make_BinningAdapter<binning::Scaled>(binning::Localization<localization::PositionY, binning::ScaledToInterval>(100));
    for (int i = 0; i < 2; ++i) {
        kalman_info.set_diffusion(i, config.diffusion() * 2.0);
        kalman_info.set_mobility(i, config.mobility());
    }
}

Output::~Output() {
}

void Output::announceStormSize(const Announcement &a) {
    if (a.group_field != input::GroupFieldSemantic::ImageNumber) {
        throw std::runtime_error("Input to emission tracker must be grouped "
                                 "by image number");
    }
    if ( ! a.position_uncertainty_x().is_given || ! a.position_uncertainty_y().is_given )
        throw std::runtime_error("Localization precision is not known to Track Emissions. Either explicitly set the "
                                 "sigmaposx and sigmaposy variables in a Expression Filter or set the optics parameters "
                                 "for counts per photons and dark current.");
    Announcement my_announcement(a);
    static_cast< dStorm::input::Traits<Localization>& >(my_announcement) 
        = dStorm::input::Traits<Localization>();
    my_announcement.in_sequence = true;
    my_announcement.position_x() = a.position_x();
    my_announcement.position_y() = a.position_y();
    my_announcement.amplitude() = a.amplitude();
    my_announcement.image_number() = a.image_number();

    dStorm::ImageTypes<2>::Size sizes;
    for (int i = 0; i < 2; ++i) {
        binners[i]->announce(a);
        binner_starts[i] = binners[i]->get_minmax().first;
        sizes[i] = int(ceil(binners[i]->get_size())) * boost::units::camera::pixel;
    }

    positional = Positional(sizes);
    while ( int(tracking.size()) < track_modulo )
        tracking.push_back( new TrackingInformation() );
    Filter::announceStormSize(my_announcement);
}

class Output::lowest_mahalanobis_distance
: public std::binary_function< TracedObject*, TracedObject*, TracedObject* > {
  public:
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

TracedObject* Output::search_closest_trace(
    const Localization &loc,
    const std::set<TracedObject*>& excluded) {
    int range[2][2];
    for (int i = 0; i < 2; ++i) {
        boost::optional<float> v = binners[i]->bin_point(loc);
        if ( ! v.is_initialized() ) return NULL;
        range[i][0] = int(floor(*v-maxDist));
        range[i][1] = int(ceil(*v+maxDist));
        for (int j = 0; j < 2; ++j) {
            range[i][j] = std::max(0, std::min( range[i][j], positional.sizes()[i].value()-1 ) );
        }
    }

    TracedObject* best = NULL;
    for (int x = range[0][0]; x <= range[0][1]; ++x)
        for (int y = range[1][0]; y <= range[1][1]; ++y)
            best = std::accumulate( 
                positional(x,y).begin(), positional(x,y).end(), 
                best, lowest_mahalanobis_distance(loc, excluded, maxDist * maxDist) );
    
    return best;
}

void
Output::update_positional( TracedObject& object ) 
{
    dStorm::Localization estimate = object.position_estimate();;
    Eigen::Array2i new_pos;
    for (int i = 0; i < 2; ++i) {
        boost::optional<float> v = binners[i]->bin_point(estimate);
        if ( ! v.is_initialized() ) return;
        new_pos[i] = round(*v);
    }

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

void
Output::receiveLocalizations(const EngineResult &er)
{
    int imNum = er.group;
    TrackingInformation &current = tracking[imNum % track_modulo];

    current.prepare( imNum );

    for (const Localization& localization : er) {
        TracedObject* trace = search_closest_trace( localization, current.emissions );
        if ( trace == NULL ) {
            std::auto_ptr<TracedObject> new_trace( new TracedObject(kalman_info, molecule_id++) );
            trace = new_trace.get();
            traced_objects.insert( new_trace );
        }

        trace->add( localization );
        update_positional( *trace );
        for (int i= 1; i < track_modulo && i <= imNum; ++i) {
            tracking[(imNum-i) % track_modulo].emissions.erase( trace );
        }
        current.emissions.insert( trace );
    }

    last_seen_frame = std::max( last_seen_frame, imNum * frame );

    if ( imNum >= track_modulo-1)
        finalizeImage(imNum - track_modulo + 1);
}

void Output::finalizeImage(int imNum) {
    TrackingInformation &data = tracking[imNum % track_modulo];

    for ( TracedObject* o : data.emissions ) {
        Filter::receiveLocalizations(o->engine_result());
        if ( o->cache_position.is_initialized() )
            positional( o->cache_position->x(), o->cache_position->y() ).erase( o );
        traced_objects.erase( *o );
    }

    data.emissions.clear();
}

void Output::store_results_( bool success ) {
    if ( success )
        for (int i = track_modulo+2; i >= 0; --i)
            if ( last_seen_frame >= i * frame )
                finalizeImage( last_seen_frame.value() - i );
    Filter::store_children_results( success );
}

}

Config::Config() 
: allowBlinking("AllowBlinking", 0 * camera::frame),
  diffusion("DiffusionConstant", boost::units::quantity<diffusion_unit>::from_value(0) ),
  mobility("Mobility", boost::units::quantity<mobility_unit>::from_value(0) ),
  distance_threshold("DistanceThreshold", 2) {}

void Config::attach_ui( simparm::NodeHandle at ) {
    distance_threshold.attach_ui( at );
    allowBlinking.attach_ui( at );
    diffusion.attach_ui( at );
    mobility.attach_ui( at );
}

std::auto_ptr<dStorm::output::OutputSource> create()
{
    return std::auto_ptr<dStorm::output::OutputSource>( 
        new dStorm::output::FilterBuilder<Output::Config,Output>() );
}

std::unique_ptr<output::Output> create_default(
    const Config& config,
    std::unique_ptr<output::Output> suboutput) {
    return make_unique<Output>(config, std::auto_ptr<output::Output>(suboutput.release()));
}

}
}
}
