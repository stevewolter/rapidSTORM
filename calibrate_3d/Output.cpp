#include "debug.h"
#include "Output.h"
#include <boost/variant/get.hpp>
#include <dStorm/polynomial_3d.h>
#include <gsl/gsl_multimin.h>
#include <dStorm/output/OutputBuilder_impl.h>
#include <boost/foreach.hpp>
#include "expression/Filter.h"
#include "expression/VariableLValue.h"
#include "expression/Variable.h"
#include "expression/QuantityDynamizer.hpp"

namespace dStorm {
namespace calibrate_3d {

class Output;

/** \todo TrueZ saves its state in the two kernel improvement field.
 *        This is a pretty dirty hack, even though it doesn't hurt for now. */
class TrueZ 
: public expression::Variable {
    expression::QuantityDynamizer< samplepos::Scalar > reader;
public:
    TrueZ() : Variable("truez") {}
    const samplepos::Scalar value(const Localization& l) const { 
        return samplepos::Scalar::from_value( l.two_kernel_improvement() );
    }

    Variable* clone() const { assert( false ); return new TrueZ(); }
    bool is_static( const input::Traits<Localization>& ) const { return false; }
    expression::DynamicQuantity get( const input::Traits<Localization>& ) const 
        { return reader.from_value( std::numeric_limits<double>::quiet_NaN() ); }
    expression::DynamicQuantity get( const Localization& l ) const 
        { return reader( value(l) ); }
    void set( input::Traits<Localization>&, const expression::DynamicQuantity& ) const {}
    bool set( const input::Traits<Localization>&, Localization& l, const expression::DynamicQuantity& v ) const { 
        l.two_kernel_improvement() = reader(v).value();
        return true;
    }
    
};

std::ostream& operator<<( std::ostream& o, const gsl_vector& v ) {
    for (size_t i = 0; i < v.size; ++i)
        o << gsl_vector_get(&v,i) << " ";
    return o;
}

Output::Output(const Config & config )
: OutputObject("Calibrate3D", "Calibrate 3D"),
  linearizer(config),
  have_set_traits_myself(false),
  terminate(false),
  fitter_finished(false),
  config( config ),
  current_volume("CurrentVolume", "Current estimation volume", 0),
  residuals("Accuracy", "Residual error", 0)
{
    std::auto_ptr<TrueZ> my_new_z( new TrueZ() );
    new_z_variable = my_new_z.get();
    parser.add_variable( std::auto_ptr<expression::Variable>(my_new_z) );
    if ( config.filter() != "" )
        filter = expression::source::make_filter( config.filter(), parser );
    if ( config.new_z() == "" )
        throw std::runtime_error("An expression for the true Z value must be given to the 3D calibrator");
    new_z_expression = expression::source::make_variable_lvalue( *new_z_variable, config.new_z(), parser );

    result_config.viewable = false;
    current_volume.viewable = false;
    residuals.viewable = false;
    result_config.registerNamedEntries();
    push_back( current_volume );
    push_back( residuals );
    push_back( result_config );
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
    localization_traits = a;
    if ( filter.get() )
        filter->announce( parser.get_variable_table(), localization_traits );
    new_z_expression->announce( parser.get_variable_table(), localization_traits );

    linearizer.set_traits( *initial_traits );

    calibration_thread = boost::thread( &Output::run_fitter, this );
    return AdditionalData(); 
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
            std::max( 0.1, gsl_vector_get( initial_position, i ) / 5 ) );

    DEBUG("Starting at position " << *initial_position);
    int success = gsl_multimin_fminimizer_set (solver, &function, 
        initial_position, initial_step_size );

    bool fitting_was_successful = false;
    /* I have no idea what is actually returned from this function, and
     * the documentation doesn't tell. */
    try {
        while ( success == GSL_SUCCESS ) {
            success = gsl_multimin_fminimizer_iterate( solver );
            DEBUG("Size is " << gsl_multimin_fminimizer_size(solver));
            double size = gsl_multimin_fminimizer_size(solver);
            current_volume = size;
            residuals = gsl_multimin_fminimizer_minimum(solver) * si::nanometre;
            current_volume.viewable = true;
            residuals.viewable = true;
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
    for (size_t i = 0; i < x->size; ++i)
        if ( gsl_vector_get(x,i) < 0 )
            return GSL_NAN;

    std::auto_ptr< engine::InputTraits > trial_traits( initial_traits->clone() );
    linearizer.delinearize( x, *trial_traits );

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
    DEBUG("Using " << localizations.size() << " localizations from " << localizations.frame_number() );
    Output::EngineResult copied_locs = localizations;
    Output::EngineResult::iterator end = copied_locs.end();
    end = new_z_expression->evaluate(
        parser.get_variable_table(), localization_traits,
        copied_locs.begin(), copied_locs.end() );
    if ( filter.get() ) 
        end = filter->evaluate( parser.get_variable_table(), localization_traits,
                                copied_locs.begin(), copied_locs.end() );
    for ( Output::EngineResult::iterator i = copied_locs.begin(); i != end; ++i ) {
        quantity<si::length> z_position = 
            (i->position().x() - 1E-6 * si::meter ) * tan( 3 * 3.1415 / 180 ) + 
            (i->position().y() - 1.2E-6 * si::meter ) / 1573.0 * 30.0;
        delta_z( quantity<si::nanolength>(i->position().z() - new_z_variable->value(*i) ).value() );
    }
}

void Output::run_finished_( const RunFinished& ) {
    DEBUG("Storing job results");
    boost::unique_lock<boost::mutex> lock( mutex );
    if ( have_set_traits_myself ) {
        double variance = boost::accumulators::variance( delta_z );
        int spots = boost::accumulators::count( delta_z );
        int missing_spots = std::max( 0, config.target_localization_number() - spots );
        position_value = sqrt( 
            ( variance * spots + 
              pow<2>( quantity<si::nanolength>(config.missing_penalty()) ).value() * missing_spots
            ) / (spots + missing_spots) );
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
            result_config.viewable = true;
            result_config.read_traits( **trial_position );
            if ( ! result_config.isActive() ) {
                (*trial_position)->print_psf_info( std::cerr << "Calibrated PSF has " ) 
                    << "; the residuals are " << residuals() << std::endl;
            }
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
        ( new output::OutputBuilder<Output>() );
}

}
}
