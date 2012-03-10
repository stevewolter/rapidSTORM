#define VERBOSE
#include "debug.h"
#include "Calibrate3D.h"
#include <simparm/Object.hh>
#include <simparm/Entry.hh>
#include <dStorm/output/Capabilities.h>
#include <boost/array.hpp>
#include <boost/optional/optional.hpp>
#include <boost/variant/get.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <dStorm/polynomial_3d.h>
#include <gsl/gsl_multimin.h>
#include <dStorm/output/OutputBuilder_impl.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/count.hpp>
#include <boost/foreach.hpp>

namespace dStorm {
namespace calibrate_3d {

std::ostream& operator<<( std::ostream& o, const gsl_vector& v ) {
    for (size_t i = 0; i < v.size; ++i)
        o << gsl_vector_get(&v,i) << " ";
    return o;
}

struct Config_ : public simparm::Object {
        Config_();
        void registerNamedEntries();
    bool can_work_with(output::Capabilities cap) 
        { return cap.test( output::Capabilities::SourceImage ); }
};

Config_::Config_()
:   simparm::Object("Calibrate3D", "Calibrate 3D on known data")
{
}

void Config_::registerNamedEntries() {
}

class Output : public output::OutputObject {
  private:
    boost::shared_ptr< const engine::InputTraits > initial_traits;
    Engine* engine;
    int variable_count;

    boost::thread calibration_thread;
    boost::mutex mutex;
    boost::condition new_job, value_computed;
    boost::optional<double> position_value;
    boost::optional< std::auto_ptr<engine::InputTraits> > trial_position;
    bool have_set_traits_myself;

    typedef boost::accumulators::accumulator_set< 
            double,
            boost::accumulators::stats< 
                boost::accumulators::tag::variance,
                boost::accumulators::tag::count > >
        Variance;
    Variance delta_z;

    void run_finished_(const RunFinished&);
    void run_fitter();
    double evaluate_function( const gsl_vector *x );
    static double gsl_callback( const gsl_vector * x, void * params )
        { return static_cast<Output*>(params)->evaluate_function(x); }

  public:
    typedef simparm::Structure<Config_> Config;

    Output(const Config &config);
    Output *clone() const { throw std::logic_error("Not implemented"); }
    ~Output();

    AdditionalData announceStormSize(const Announcement &);
    RunRequirements announce_run(const RunAnnouncement&);
    void receiveLocalizations(const EngineResult&);
    void store_results() {}

    const char *getName() { return "Calibrate 3D"; }
};

Output::Output(const Config &)
: OutputObject("Calibrate3D", "Calibrate 3D"),
  have_set_traits_myself(false)
{
}

Output::RunRequirements Output::announce_run(const RunAnnouncement&)
{ 
    DEBUG("New run is announced");
    delta_z = Variance(); 
    return RunRequirements().set(MayNeedRestart); 
}

Output::AdditionalData Output::announceStormSize(const Output::Announcement &a)
{ 
    DEBUG("Initial data are announced");
    if ( ! a.input_image_traits.get() )
        throw std::runtime_error("Source images are not given to 3D calibrator");
    initial_traits.reset( a.input_image_traits->clone() );
    engine = a.engine;

    variable_count = 1;
    variable_count += initial_traits->plane_count() * 2;

    calibration_thread = boost::thread( &Output::run_fitter, this );
    return AdditionalData(); 
}

Output::~Output() {}

void Output::run_fitter()
{
    DEBUG("Running non-linear fitter");
    gsl_multimin_fminimizer* solver;
    gsl_multimin_function function;

    solver = gsl_multimin_fminimizer_alloc( 
        gsl_multimin_fminimizer_nmsimplex2, variable_count );
    function.f = &Output::gsl_callback;
    function.n = variable_count;
    function.params = this;

    gsl_vector* initial_position = gsl_vector_alloc( variable_count );
    gsl_vector* initial_step_size = gsl_vector_alloc( variable_count );

    const int fluorophore = 0;
    int cur = 0;
    for (int plane_index = 0; plane_index < initial_traits->plane_count(); ++plane_index) {
        const traits::Optics& plane = initial_traits->optics(plane_index);
        for (Direction d = Direction_X; d < Direction_2D; ++d) {
            gsl_vector_set( initial_position, cur, 
                (*plane.psf_size(fluorophore))[d] / (1E-6 * si::meter) );
            gsl_vector_set( initial_step_size, cur, 0.5 );
        }
        ++cur;

        for (Direction d = Direction_X; d < Direction_2D; ++d) {
            gsl_vector_set( initial_position, cur, 
                (*plane.z_position)[d] / (1E-6 * si::meter) );
            gsl_vector_set( initial_step_size, cur, 1.0 );
        }
        ++cur;
    }

    const dStorm::traits::Polynomial3D& p3d = 
        boost::get<dStorm::traits::Polynomial3D>(*initial_traits->depth_info);
    for (int term = polynomial_3d::FirstTerm; term <= polynomial_3d::LastTerm; ++term) {
        if ( term == 2 ) {
            for (Direction d = Direction_X; d < Direction_2D; ++d) {
                gsl_vector_set( initial_position, cur, 
                    p3d.get_slope( d, term ) / (1E-6 * si::meter) );
                gsl_vector_set( initial_step_size, cur, 0.5 );
            }
            ++cur;
        }
    }
    assert( cur == variable_count );

    DEBUG("Starting at position " << *initial_position);
    int success = gsl_multimin_fminimizer_set (solver, &function, 
        initial_position, initial_step_size );

    /* I have no idea what is actually returned from this function, and
     * the documentation doesn't tell. */
    while ( success == GSL_SUCCESS ) {
        success = gsl_multimin_fminimizer_iterate( solver );
        DEBUG("Size is " << gsl_multimin_fminimizer_size(solver));
        if ( gsl_multimin_fminimizer_size(solver) < 1E-4 )
            break;
    }

    {
        boost::lock_guard<boost::mutex> lock( mutex );
        trial_position = boost::in_place();
        new_job.notify_all();
    }
    
    gsl_vector_free( initial_position );
    gsl_vector_free( initial_step_size );
    gsl_multimin_fminimizer_free(solver); 
}

double Output::evaluate_function( const gsl_vector *x ) {
    DEBUG("Callback by GSL to evaluate position " << *x);
    std::auto_ptr< engine::InputTraits > trial_traits
        ( initial_traits->clone() );

    for (size_t i = 0; i < x->size; ++i)
        if ( gsl_vector_get(x,i) < 0 )
            return GSL_NAN;

    const int fluorophore = 0;
    int cur = 0;
    for (int plane_index = 0; plane_index < initial_traits->plane_count(); ++plane_index) {
        traits::Optics& plane = trial_traits->optics(plane_index);
        for (Direction d = Direction_X; d < Direction_2D; ++d)
            (*plane.psf_size(fluorophore))[d] = 
                gsl_vector_get( x, cur ) * (1E-6 * si::meter);
        ++cur;
        for (Direction d = Direction_X; d < Direction_2D; ++d)
            (*plane.z_position)[d] =
                gsl_vector_get( x, cur ) * (1E-6 * si::meter);
        ++cur;
        DEBUG("Z center of plane " << plane_index << " is " << plane.z_position->transpose());
    }

    traits::Polynomial3D& p3d = boost::get<traits::Polynomial3D>(*trial_traits->depth_info);
    for (int term = polynomial_3d::FirstTerm; term <= polynomial_3d::LastTerm; ++term) {
        if ( term == 2 ) {
            for (Direction d = Direction_X; d < Direction_2D; ++d) 
                p3d.set_slope( d, term, gsl_vector_get( x, cur ) * 1E-6 * si::meter );
            ++cur;
        }
    }

    assert( cur == variable_count );

    {
        boost::unique_lock<boost::mutex> lock( mutex );
        DEBUG("Publishing trial position");
        trial_position = boost::in_place();
        *trial_position = trial_traits;
        position_value.reset();
        new_job.notify_all();
        while ( ! position_value )
            value_computed.wait( lock );
        DEBUG("Waiting for result");
        return *position_value;
    }
}

void Output::receiveLocalizations(const Output::EngineResult& localizations )
{
    DEBUG("Using " << localizations.size() << " localizations from " << localizations.frame_number() );
    BOOST_FOREACH( const Localization& loc, localizations ) {
        quantity<si::length> z_position = 
            (loc.position().x() - 1E-6 * si::meter ) * tan( 3 * 3.1415 / 180 ) + 
            (loc.position().y() - 1.2E-6 * si::meter ) / 1573.0 * 30.0;
        if ( z_position >= 8E-7 * si::meter && z_position <= 2.3E-6 * si::meter )
            delta_z( (loc.position().z() - z_position) / (1E-9 * si::meter) );
    }
}

void Output::run_finished_( const RunFinished& ) {
    DEBUG("Storing job results");
    boost::unique_lock<boost::mutex> lock( mutex );
    if ( have_set_traits_myself ) {
        double variance = boost::accumulators::variance( delta_z );
        int spots = boost::accumulators::count( delta_z );
        int missing_spots = std::max( 0, 371 - spots );
        position_value = sqrt( (variance * spots + 2E6 * missing_spots) / (spots + missing_spots) );
        DEBUG("Position's value is " << *position_value );
        value_computed.notify_all();
    }
    DEBUG("Getting next trial position");
    while ( ! trial_position )
        new_job.wait(lock);
    if ( trial_position->get() ) {
        DEBUG("Restarting engine");
        engine->change_input_traits( std::auto_ptr< input::BaseTraits >(trial_position->release()) );
        engine->restart();
        have_set_traits_myself = true;
        trial_position.reset();
    }
}

std::auto_ptr< output::OutputSource >
make_output_source() {
    return std::auto_ptr< output::OutputSource >
        ( new output::OutputBuilder<Output>() );
}

}
}
