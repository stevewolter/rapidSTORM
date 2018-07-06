#include "debug.h"
#include "calibrate_3d/Output.h"
#include <boost/variant/get.hpp>
#include "polynomial_3d.h"
#include <gsl/gsl_multimin.h>
#include "output/OutputBuilder_impl.h"
#include <boost/foreach.hpp>
#include "calibrate_3d/ZTruth.h"
#include "threed_info/Polynomial3D.h"
#include <boost/units/cmath.hpp>
#include <boost/utility/in_place_factory.hpp>

namespace dStorm {
namespace calibrate_3d {

class Output;

std::ostream& operator<<( std::ostream& o, const gsl_vector& v ) {
    for (size_t i = 0; i < v.size; ++i)
        o << gsl_vector_get(&v,i) << " ";
    return o;
}

Output::Output(const Config & config )
: z_truth( config.get_z_truth() ),
  linearizer(config),
  engine(NULL),
  result_config( traits::PlaneConfig::PSFDisplay ),
  have_set_traits_myself(false),
  terminate(false),
  fitter_finished(false),
  config( config ),
  current_volume("CurrentVolume", "Current estimation volume", 0),
  residuals("Accuracy", "Residual error", 0)
{
    result_config.hide();
    current_volume.hide();
    residuals.hide();
}

void Output::attach_ui_( simparm::NodeHandle at ) {
    current_volume.attach_ui( at );
    residuals.attach_ui( at );
    result_config.attach_ui( at );
}

Output::RunRequirements Output::announce_run(const RunAnnouncement&)
{ 
    DEBUG("New run is announced");
    found_spots = 0;
    squared_errors = 0 * si::meter * si::meter;
    return RunRequirements().set(MayNeedRestart); 
}

void Output::announceStormSize(const Output::Announcement &a)
{ 
    DEBUG("Initial data are announced");
    if ( ! a.input_image_traits.get() )
        throw std::runtime_error("Source images are not given to 3D calibrator");
    for ( engine::InputTraits::const_iterator i = a.input_image_traits->begin();
            i != a.input_image_traits->end(); ++i )
        for (Direction dir = Direction_First; dir != Direction_2D; ++dir)
            if ( dynamic_cast<const threed_info::Polynomial3D*>( i->optics.depth_info(dir).get() ) == NULL )
                throw std::runtime_error("3D calibration output works only with polynomial 3D");
    initial_traits.reset( a.input_image_traits->clone() );
    engine = a.engine;

    z_truth->set_meta_info( a );
    linearizer.set_traits( *initial_traits );

    calibration_thread = boost::thread( &Output::run_fitter, this );
}

Output::~Output() {
    if ( calibration_thread.joinable() ) {
        terminate = true;
        value_computed.notify_all();
        calibration_thread.join();
    }
}

void Output::run_fitter()
{
    DEBUG("Running non-linear fitter");
    gsl_multimin_fminimizer* solver;
    gsl_multimin_function function;

    const int variable_count = linearizer.parameter_count();
    solver = gsl_multimin_fminimizer_alloc( 
        gsl_multimin_fminimizer_nmsimplex2, variable_count );
    function.f = &Output::gsl_callback;
    function.n = variable_count;
    function.params = this;

    gsl_vector* initial_position = gsl_vector_alloc( variable_count );
    gsl_vector* initial_step_size = gsl_vector_alloc( variable_count );

    linearizer.linearize( *initial_traits, initial_position );
    for (size_t i = 0; i < initial_position->size; ++i)
        gsl_vector_set( initial_step_size, i,
            std::max(config.absolute_initial_step(),
                std::abs(gsl_vector_get( initial_position, i )) * config.relative_initial_step()) );

    DEBUG("Starting at position " << *initial_position);
    int success = gsl_multimin_fminimizer_set (solver, &function, 
        initial_position, initial_step_size );

    bool fitting_was_successful = false;
    try {
        /* I have no idea what is actually returned from gsl_multimin_fminimizer_set, and
        * the documentation doesn't tell. */
        while ( success == GSL_SUCCESS ) {
            success = gsl_multimin_fminimizer_iterate( solver );
            DEBUG("Size is " << gsl_multimin_fminimizer_size(solver));
            double size = gsl_multimin_fminimizer_size(solver);
            current_volume = size;
            residuals = gsl_multimin_fminimizer_minimum(solver) * si::nanometre;
            current_volume.show();
            residuals.show();
            if ( size < config.target_volume() ) {
                fitting_was_successful = true;
                break;
            }
        }
    } catch ( boost::thread_interrupted ) {
        /* Do nothing. The rest of this function is cleanup. */
    }

    boost::lock_guard<boost::mutex> lock( mutex );
    trial_position = boost::in_place();
    if ( fitting_was_successful ) {
        DEBUG("Pushing final position");
        *trial_position = std::auto_ptr< engine::InputTraits >( initial_traits->clone() );
        linearizer.delinearize( gsl_multimin_fminimizer_x(solver), **trial_position );
    }
    fitter_finished = true;
    new_job.notify_all();
    
    gsl_vector_free( initial_position );
    gsl_vector_free( initial_step_size );
    gsl_multimin_fminimizer_free(solver); 
}

double Output::evaluate_function( const gsl_vector *x ) {
    DEBUG("Callback by GSL to evaluate position " << *x);
    std::auto_ptr< engine::InputTraits > trial_traits( initial_traits->clone() );
    if (!linearizer.delinearize( x, *trial_traits )) {
        return GSL_NAN;
    }

    boost::unique_lock<boost::mutex> lock( mutex );
    DEBUG("Publishing trial position");
    trial_position = boost::in_place();
    *trial_position = trial_traits;
    new_job.notify_all();

    position_value.reset();
    while ( ! position_value && ! terminate )
        value_computed.wait( lock );
    if ( terminate ) throw boost::thread_interrupted();
    DEBUG("Waiting for result");
    return *position_value;
}

void Output::receiveLocalizations(const Output::EngineResult& localizations )
{
    Output::EngineResult copied_locs = localizations;
    Output::EngineResult::iterator begin = copied_locs.begin(), end = z_truth->calibrate( copied_locs );
    for ( Output::EngineResult::iterator i = begin; i != end; ++i ) {
        ++found_spots;
        squared_errors += pow<2>( i->position().z() - z_truth->true_z(*i) );
    }
}

void Output::run_finished_( const RunFinished& ) {
    DEBUG("Storing job results");
    boost::unique_lock<boost::mutex> lock( mutex );
    if ( have_set_traits_myself ) {
        double missing_spots = std::max( 0.0, double(config.target_localization_number() - found_spots) );
        DEBUG("Localized " << found_spots << " spots with SD " << sqrt(squared_errors/double(found_spots)) << " and have " << missing_spots << " missing spots");
        quantity<si::length> missing_penalty( config.missing_penalty() );
        quantity<si::length> rmse = sqrt( 
            ( squared_errors + 
              pow<2>( missing_penalty ) * missing_spots
            ) / (found_spots + missing_spots) );
        position_value = quantity<si::nanolength>(rmse).value();
        DEBUG("Position's value is " << *position_value );
        value_computed.notify_all();
    }
    DEBUG("Getting next trial position");
    while ( ! trial_position && ! fitter_finished )
        new_job.wait(lock);
    if ( trial_position && trial_position->get() ) {
        DEBUG("Restarting engine");
        if ( fitter_finished ) {
            DEBUG("Putting final position into config");
            result_config.read_traits( **trial_position );
            if ( ! result_config.ui_is_attached() ) {
                (*trial_position)->print_psf_info( std::cerr << "Calibrated PSF has " ) 
                    << "; the residuals are " << residuals() << std::endl;
            }
            result_config.show();
        }
        engine->change_input_traits( std::auto_ptr< input::BaseTraits >(trial_position->release()) );
        engine->restart();
        have_set_traits_myself = true;
        trial_position.reset();
    }
}

std::auto_ptr< output::OutputSource >
make_output_source() {
    return std::auto_ptr< output::OutputSource >
        ( new output::OutputBuilder<Config,Output>() );
}

}
}
