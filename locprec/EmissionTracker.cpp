#include "EmissionTracker.h"
#include <algorithm>

using namespace dStorm;
using namespace std;

namespace locprec {

int traceNumber = 0;
static int number_of_emission_nodes = 0;

template <typename T>
T sq(const T& a) { return a*a; }

double distSq(const Localization& e, const Localization &loc) {
    return ( sq(e.x() - loc.x()) + sq(e.y() - loc.y()) );
}

struct EmissionTracker::TrackingInformation {
    int imageNumber;
    cimg_library::CImg<List::iterator> positional;
    List emissions;
    data_cpp::Vector<dStorm::Localization> output;
    Eigen::Vector2d displacement;

    TrackingInformation(
        List::Allocator &a, int width, int height
    ) : positional(width, height), emissions(a)
        { imageNumber = -5; }

    void prepare(int imageNumber ) {
        this->imageNumber = imageNumber;
        memset(positional.ptr(), 0, 
                positional.size() * sizeof(List::iterator));
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

EmissionTracker::TracedObject::TracedObject(const EmissionTracker& papa)

: KalmanTrace<2>(papa.measurement_covar, papa.random_system_dynamics_covar)
{
}

EmissionTracker::_Config::_Config() 
: simparm::Object("EmissionTracker", "Track emissions"),
  allowBlinking("AllowBlinking", "Allow fluorophores to skip n frames"),
  expectedDeviation("ExpectedDeviation", "SD of expected distance between tracked localizations", 0.1)
{
}

void EmissionTracker::_Config::registerNamedEntries()

{
    push_back( allowBlinking );
    push_back( expectedDeviation );
    push_back( reducer );
}

EmissionTracker::EmissionTracker( 
    const Config &config,
    std::auto_ptr<Output> output )
: Object("EmissionTracker", "Emission tracking status"),
  track_modulo(5), next_to_track_moved(mutex), 
  garbage_bin(allocator),
  tracking(track_modulo, (TrackingInformation*)NULL),
  measurement_covar( Eigen::Matrix<double,2,2>::Identity() * 
                     pow(config.expectedDeviation(), 2) ),
  random_system_dynamics_covar( Eigen::Matrix<double,4,4>::Identity() * 
                                pow(0.0f, 2) ),
  reducer( config.reducer.make_trace_reducer() ), 
  target(output),
  maxDist(2*sqrt(2)*config.expectedDeviation())
{
    push_back(*target);
}

EmissionTracker::~EmissionTracker() {
    for (int i = 0; i < track_modulo; i++)
        if ( tracking[i] != NULL) {
            delete tracking[i];
            tracking[i] = NULL;
        }
}

Output::AdditionalData
EmissionTracker::announceStormSize(const Announcement &a) 

{
    ost::MutexLock lock(mutex);
    image_width = a.width; image_height = a.height; 
    movie_length = a.length; 

    stopped = false;
    next_to_track = 0;
    for (int i = 0; i < track_modulo; i++) {
        if (tracking[i] == NULL)
            tracking[i] = new TrackingInformation
                (allocator, image_width, image_height);
    }
    AdditionalData childData = target->announceStormSize(a);
    Output::check_additional_data_with_provided
        ( simparm::Object::name(), LocalizationSources,
            childData );
    return NoData;
}

EmissionTracker::List::iterator
EmissionTracker::link(const Localization &loc, const int imNum)
{
    const int cell_x = int(round(loc.x())), 
              cell_y = int(round(loc.y()));

    double max_sq_distance = 4, 
           best_distance = std::numeric_limits<double>::max(),
           hope_threshold = 16;
    int current_result_size = 0;
    List::iterator result(NULL);
    const int backtrack = std::min( imNum+1, track_modulo );

    for ( int i = 1; i < backtrack; i++ ) {
        TrackingInformation &cell = *tracking[ (imNum-i) % track_modulo ];
        List::iterator candidate = cell.positional(cell_x, cell_y);
        if ( candidate == result || !candidate ||
             candidate->has_lost_hope(i) )
            continue;
        else {
            double sq_distance = candidate->sq_distance_in_sigmas( loc );
            if ( sq_distance < max_sq_distance && 
                 sq_distance < best_distance ) 
            {
                if ( current_result_size < 2 || candidate->size() > 1 ) {
                    best_distance = sq_distance;
                    result = candidate;
                    current_result_size = candidate->size();
                }
            } else if ( sq_distance < hope_threshold ) {
                candidate->make_hope();
            }
        }
    }
    
    return result;
}

EmissionTracker::List::iterator 
EmissionTracker::start_trace(const Localization&) 
{
    List::iterator e;
    /* Need a new emission node. Get one from the garbage bin
        * and re-initialize it. */
    e = getNewEmissionNode();
    return e;
}

void
EmissionTracker::write_positional_image(
    const TracedObject& trace,
    List::iterator to_write
) 
{
    int imNum = trace.getPosition();
    TrackingInformation &current = *tracking[imNum % track_modulo];

    double cx = trace.getEstimate_X(), cy = trace.getEstimate_Y();
    int nplx = max(0, int(floor(cx-maxDist))), 
        nphx = int(ceil(cx+maxDist)), 
        nply = max(0, int(floor(cy-maxDist))),
        nphy = int(ceil(cy+maxDist));

    nphx = min( nphx, int(current.positional.width-1) );
    nphy = min( nphy, int(current.positional.height-1) );
    
    for (int x = nplx; x <= nphx; x++)
        for (int y = nply; y <= nphy; y++)
            current.positional(x,y) = to_write;
}

Output::Result
EmissionTracker::receiveLocalizations(const EngineResult &er)
{
    ost::MutexLock lock(mutex);
    if (stopped) return KeepRunning;

    int imNum = er.forImage;
    TrackingInformation &current = *tracking[imNum % track_modulo];

    /* Wait for the tracking to arrive at the current position. */
    while (next_to_track != imNum && !stopped) {
        next_to_track_moved.wait();
    }
    if (stopped) return KeepRunning;

    current.prepare( imNum );

    for (int loc = 0; loc < er.number; loc++) 
    {
        const Localization *locs = er.first + loc;
        List::iterator trace = link( *locs, imNum );
        if ( trace ) 
            write_positional_image( *trace, List::iterator(NULL) );
        else
            trace = start_trace( *locs );

        trace->add( *locs );
        List::splice(current.emissions.end(), trace);
        write_positional_image( *trace, trace );
    }

    if ( imNum >= track_modulo-1)
        finalizeImage(imNum - track_modulo + 1);

    next_to_track = imNum+1;
    next_to_track_moved.signal();
    return KeepRunning;
}

void EmissionTracker::finalizeImage(int imNum) {
    TrackingInformation &data = *tracking[imNum % track_modulo];
    List& ended = data.emissions;
    data_cpp::Vector<Localization>& results = data.output;

    results.clear();
    for ( List::const_iterator i = ended.begin(), end = ended.end();
          i != end; i++ ) 
    {
        reducer->reduce_trace_to_localization( 
            *i, results.allocate(), Eigen::Vector2d::Zero() );
        results.commit();
    }

    EngineResult er;
    er.forImage = imNum;
    er.first = results.ptr();
    er.number = results.size();

    target->receiveLocalizations(er);

    List::splice( garbage_bin.begin(), ended.begin(), ended.end() );

    assert( ended.empty() );
    assert( TrackingInformation::all_nodes_accounted_for(tracking, garbage_bin.size()) );
}

void EmissionTracker::propagate_signal(ProgressSignal s) {
    ost::MutexLock lock(mutex);
    if ( s == Engine_run_is_aborted ) {
        stopped = true;
        next_to_track_moved.signal();
    } else if ( s == Engine_is_restarted ) {
        stopped = false;
        next_to_track = 0;
    } else if ( s == Engine_run_succeeded ) {
        for (int i = std::max(0, next_to_track-(track_modulo-1));
                 i < next_to_track; i++)
            finalizeImage(i); 
    }
    if ( target.get() != NULL )
        target->propagate_signal(s);
}

EmissionTracker::List::iterator EmissionTracker::getNewEmissionNode()

{
    if (garbage_bin.empty()) {
        garbage_bin.push_back( List::value_type(*this) );
        number_of_emission_nodes++;
        return garbage_bin.begin();
    } else {
        garbage_bin.begin()->clear();
        return garbage_bin.begin();
    }
}

}
