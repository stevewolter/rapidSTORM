#include "kalman_filter/fwd.h"

#include "simparm/BoostUnits.h"
#include "simparm/Entry.h"
#include "simparm/Entry.h"
#include "simparm/FileEntry.h"
#include "output/TraceReducer.h"
#include <cassert>
#include <Eigen/Core>
#include <vector>
#include "kalman_filter/KalmanTrace.h"
#include "output/FilterBuilder.h"
#include "binning/binning.h"
#include "binning/binning.hpp"
#include "UnitEntries/Nanometre.h"
#include <boost/ptr_container/ptr_set.hpp>
#include <boost/ptr_container/ptr_array.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional/optional.hpp>
#include <algorithm>
#include <numeric>
#include <boost/units/Eigen/Array>
#include "helpers/back_inserter.h"
#include "binning/localization_impl.h"
#include "output/Filter.h"
#include "image/iterator.h"
#include <boost/units/Eigen/Array>
#include <boost/utility/in_place_factory.hpp>

using namespace std;
using namespace boost::units::camera;

namespace dStorm {
namespace kalman_filter {

template <typename Type>
struct address_is_less : public std::binary_function<Type,Type,bool> {
    bool operator()( const Type& a, const Type& b ) const { return &a < &b; };
};

class Output 
: public dStorm::output::Filter
{
public:
    class Config;

private:
    class lowest_mahalanobis_distance;
    class TracedObject : public KalmanTrace<2> {
      public:
        boost::optional<Eigen::Vector2i> cache_position;
        TracedObject(const Output &papa);
        ~TracedObject();

        void add( const dStorm::Localization& l) {
            KalmanTrace<2>::add(l);
        }
    };

    std::auto_ptr<binning::Scaled> binners[2];
    float binner_starts[2];
    typedef dStorm::Image< std::set<TracedObject*>, 2 > Positional;
    Positional positional;

    const int track_modulo;
    frame_index last_seen_frame;

    boost::ptr_set< TracedObject, address_is_less<TracedObject> > traced_objects;

    struct TrackingInformation;
    boost::ptr_vector<TrackingInformation> tracking;

    KalmanMetaInfo<2> kalman_info;

    TracedObject* search_closest_trace(
        const dStorm::Localization &loc, 
        const std::set<TracedObject*>& excluded);
    void update_positional( TracedObject& object );
    void finalizeImage(int i);

    std::auto_ptr<dStorm::output::TraceReducer> reducer;

    const double maxDist;
    void store_results_( bool success ); 

public:
    Output( const Config& config,
                        std::auto_ptr<dStorm::output::Output> output );
    ~Output();
    Output( const Output& );
    Output *clone() const
        {throw std::logic_error("Emission tracker is not cloneable.");}
    Output& operator=( const Output& );

    AdditionalData announceStormSize(const Announcement &);
    RunRequirements announce_run(const RunAnnouncement &);
    void receiveLocalizations(const EngineResult &);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class Output::Config  {
public:
    Config();
    void attach_ui( simparm::NodeHandle at );
    static std::string get_name() { return "EmissionTracker"; }
    static std::string get_description() { return "Track emissions"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }

    simparm::Entry< quantity<camera::time,int> > allowBlinking;
    simparm::Entry< boost::units::quantity<KalmanMetaInfo<2>::diffusion_unit> > diffusion;
    simparm::Entry< boost::units::quantity<KalmanMetaInfo<2>::mobility_unit> > mobility;
    dStorm::output::TraceReducer::Config reducer;
    simparm::Entry<float> distance_threshold;

    bool determine_output_capabilities
        ( dStorm::output::Capabilities& cap ) 
    { 
        cap.set_intransparency_for_source_data();
        cap.set_cluster_sources( true );
        return true;
    }
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

Output::TracedObject::TracedObject(const Output& papa)
: KalmanTrace<2>(papa.kalman_info)
{
}
Output::TracedObject::~TracedObject() {
}

Output::Config::Config() 
: allowBlinking("AllowBlinking", 0 * camera::frame),
  diffusion("DiffusionConstant", boost::units::quantity<KalmanMetaInfo<2>::diffusion_unit>::from_value(0) ),
  mobility("Mobility", boost::units::quantity<KalmanMetaInfo<2>::mobility_unit>::from_value(0) ),
  distance_threshold("DistanceThreshold", 2)
{
}

void Output::Config::attach_ui( simparm::NodeHandle at )
{
    distance_threshold.attach_ui( at );
    allowBlinking.attach_ui( at );
    diffusion.attach_ui( at );
    mobility.attach_ui( at );
    reducer.attach_ui( at );
}

Output::Output( 
    const Config &config,
    std::auto_ptr<dStorm::output::Output> output )
: Filter(output),
  track_modulo( config.allowBlinking().value()+2 ), 
  last_seen_frame( 0 * camera::frame ),
  reducer( config.reducer.make_trace_reducer() ), 
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

output::Output::AdditionalData
Output::announceStormSize(const Announcement &a) 
{
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
    my_announcement.source_traits.push_back( 
        boost::shared_ptr< dStorm::input::Traits<Localization> >( new dStorm::input::Traits<Localization>(a) ) );

    dStorm::ImageTypes<2>::Size sizes;
    for (int i = 0; i < 2; ++i) {
        binners[i]->announce(a);
        binner_starts[i] = binners[i]->get_minmax().first;
        sizes[i] = int(ceil(binners[i]->get_size())) * boost::units::camera::pixel;
    }

    positional = Positional(sizes);
    while ( int(tracking.size()) < track_modulo )
        tracking.push_back( new TrackingInformation() );
    AdditionalData childData = Filter::announceStormSize(my_announcement);
    Output::check_additional_data_with_provided
        ( Config::get_name(), AdditionalData().set_cluster_sources(),
            childData );
    return AdditionalData();
}

Output::RunRequirements Output::announce_run(const RunAnnouncement & a) {
    return Filter::announce_run(a);
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
    dStorm::Localization estimate;
    Eigen::Vector2f position_estimate = object.getPositionEstimate().cast<float>();
    estimate.position_x() = position_estimate.x() * si::meter;
    estimate.position_y() = position_estimate.y() * si::meter;
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

    last_seen_frame = std::max( last_seen_frame, imNum * frame );

    if ( imNum >= track_modulo-1)
        finalizeImage(imNum - track_modulo + 1);
}

void Output::finalizeImage(int imNum) {
    TrackingInformation &data = tracking[imNum % track_modulo];

    EngineResult er;
    er.group = imNum;

    for ( std::set<TracedObject*>::const_iterator i = data.emissions.begin(), end = data.emissions.end();
          i != end; i++ ) 
    {
        TracedObject& o = **i;
        reducer->reduce_trace_to_localization( 
            o.begin(), o.end(),
            boost::back_inserter(er), 
            dStorm::samplepos::Constant(0*si::meter) );
        if ( o.cache_position.is_initialized() )
            positional( o.cache_position->x(), o.cache_position->y() ).erase( &o );
        traced_objects.erase( o );
    }

    data.emissions.clear();
    Filter::receiveLocalizations(er);
}

void Output::store_results_( bool success ) {
    if ( success )
        for (int i = track_modulo+2; i >= 0; --i)
            if ( last_seen_frame >= i * frame )
                finalizeImage( last_seen_frame.value() - i );
    Filter::store_children_results( success );
}

std::auto_ptr<dStorm::output::OutputSource> create()
{
    return std::auto_ptr<dStorm::output::OutputSource>( 
        new dStorm::output::FilterBuilder<Output::Config,Output>() );
}

}
}
