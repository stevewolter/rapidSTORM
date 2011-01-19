#include "LocalizationFilter.h"
#include <limits>
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/doc/context.h>
#include <Eigen/Array>
#include <dStorm/unit_matrix_operators.h>
#include <simparm/UnitEntry_Impl.hh>
#include <simparm/OptionalEntry_impl.hh>

#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/systems/si/prefixes.hpp>
#include <boost/units/systems/camera/time.hpp>

#include "debug.h"
#include <dStorm/error_handler.h>
#include <string>

#include <dStorm/traits/range_impl.h>

namespace boost {
namespace units {

std::string name_string(const dStorm::output::LocalizationFilter::ShiftSpeed&) {
    return "picometer per second";
}
std::string symbol_string(const dStorm::output::LocalizationFilter::ShiftSpeed&)
{
    return "pm/s";
}
    
}
}
using namespace boost::units;

namespace dStorm {
namespace output {

class LocalizationFilter::ReEmitter 
: public ost::Thread, public ResultRepeater
{
    /** Flag indicating whether any parameter changed. This
      * flag will only be set to false in the subthread, thus
      * not needing a mutex. */
    bool parameter_changed;
    /** Mutex for the condition. */
    ost::Mutex mutex;
    /** Condition that indicates change in parameter_changed
      * or need_re_emitter. */
    ost::Condition condition;
    /** This flag will be set to false when the re_emitter should
      * terminate for destruction of the LocalizationFilter. */
    bool need_re_emitter;

    LocalizationFilter &work_for;

  public:
    ReEmitter(LocalizationFilter& work_for) :
        ost::Thread("Localization re-emitter"),
        parameter_changed(0),
        condition(mutex),
        need_re_emitter(true),
        work_for(work_for)
    {
        ost::WriteLock lock( work_for.emissionMutex );
        work_for.output->propagate_signal( Engine_run_is_starting );
    }

    ~ReEmitter() { 
        DEBUG("Destructing Reemitter");
        need_re_emitter = false; 
        condition.signal();
        DEBUG("Joining subthread");
        join(); 
        DEBUG("Joined subthread");
    }

    void run() throw()
    {
      DEBUG("Running localization reemitter");
      try {
        DEBUG("Acquiring mutex");
        mutex.enterMutex();
        DEBUG("Acquired mutex");
        while (need_re_emitter) {
            DEBUG("Running loop");
            if ( parameter_changed ) {
                parameter_changed = false;
                mutex.leaveMutex();
                work_for.reemit_localizations( parameter_changed );
                mutex.enterMutex();
            }
            if (!parameter_changed && need_re_emitter) {
                DEBUG("Waiting for next iteration");
                condition.wait();
                DEBUG("Waited for next iteration");
            }
        }
        mutex.leaveMutex();
      } catch (const std::bad_alloc& e) {
        std::cerr << "Ran out of memory." << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "An error occured during result recomputation: "
                  << e.what() << std::endl;
      } catch (...) {
        std::cerr << "An unknown error occured during result recomputation."
                  << std::endl;
      }
      DEBUG("Finished reemitter subthread");
    }

    void repeat_results() {
        parameter_changed = true;
        condition.signal();
    }
};

LocalizationFilter::LocalizationFilter(
    const Config& c,
    std::auto_ptr<output::Output> output
)
    : OutputObject("AF", "LocalizationFilter"),
      simparm::Node::Callback( simparm::Event::ValueChanged ),
      localizationsStore( new output::Localizations() ),
      from(c.from), to(c.to), shift_scale(c.shift_scale),
      x_shift(c.x_shift), y_shift(c.y_shift), 
      two_kernel_significance(c.two_kernel_significance),
      output(output)
{ 
    shift_velocity.fill( AppliedSpeed::from_value(0) );
    init();
}

LocalizationFilter::LocalizationFilter(const LocalizationFilter& o)
: OutputObject(o),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  localizationsStore( new output::Localizations(o.localizationsStore.getResults()) ), 
  from(o.from), to(o.to), shift_scale(o.shift_scale),
  x_shift(o.x_shift), y_shift(o.y_shift),
  two_kernel_significance(o.two_kernel_significance),
  output( o.output->clone() )
{
    init();
}

LocalizationFilter::~LocalizationFilter()
{
    DEBUG("Destructing LF");
}

void LocalizationFilter::init()
{
    v_from = from();
    v_to = to();

    receive_changes_from( from.value );
    receive_changes_from( to.value );
    receive_changes_from( shift_scale.value );
    receive_changes_from( x_shift.value );
    receive_changes_from( y_shift.value );
    receive_changes_from( two_kernel_significance.value );

    re_emitter.reset( new ReEmitter( *this ) );
    re_emitter->start();

    push_back( from );
    push_back( to );
    push_back( shift_scale );
    push_back( x_shift );
    push_back( y_shift );
    push_back( two_kernel_significance );

    if ( output.get() != NULL )
        push_back( *output );
}

void LocalizationFilter::reemit_localizations(bool& terminate) {
    ost::MutexLock lock_a( locStoreMutex );
    ost::WriteLock lock_b( emissionMutex );
    if ( outputState == Running )
        output->propagate_signal( Output::Engine_run_is_aborted );
    if ( outputState != PreStart )
        output->propagate_signal( Output::Engine_is_restarted );

    const Localizations& localizations = localizationsStore.getResults();
    
    bool continued = false;
    int continue_from = 0;
    for ( int bin = 0; bin < localizations.binNumber(); bin++ ) 
    {
        const Localization* bindat = localizations.getBin(bin);
        int len = localizations.sizeOfBin(bin);
        /* Start gives start of current continuous region, end the 
         * position just after the end. */
        int start = 0, end = 1;
        while ( start < len ) {
            /* Seek end of continuous region with the end pointer. */
            while ( end < len && bindat[start].frame_number() ==
                                 bindat[end].frame_number() )
                end++;

            bool continued_after;
            if ( end == len ) {
                int nextBin = bin+1;
                continued_after = 
                    nextBin < localizations.binNumber() &&
                    localizations.sizeOfBin(nextBin) > 0 &&
                    localizations.getBin(nextBin)[0].frame_number()
                        == bindat[start].frame_number();
                continue_from = start;
            } else {
                continued_after = false;
            }

            if ( !continued_after ) {
              emit_localizations( 
                /* If continued, output range in last bin */
                (continued) 
                    ? ( localizations.getBin(bin-1) + continue_from )
                    : NULL,
                (continued)
                    ? ( localizations.sizeOfBin(bin-1) - continue_from )
                    : 0,
                bindat[start].frame_number(),
                /* Output currently selected range. */
                bindat + start, end-start 
              );
            }

            continued = continued_after;
            start = end;
            
            if ( terminate || ErrorHandler::global_termination_flag() ) {
                output->propagate_signal( Engine_run_is_aborted );
                return;
            }
        }
    }

    if ( inputState == Succeeded ) {
        output->propagate_signal( Engine_run_succeeded );
        outputState = Succeeded;
    }
}

void LocalizationFilter::copy_and_modify_localizations(
    const Localization *from, int n, 
    Localization *to, int& to_count )

{
    for ( int i = 0; i < n; i++ ) {
        amplitude strength = from[i].amplitude();
        if ( (!v_from.is_set() || strength >= *v_from) &&
             (!v_to.is_set() || strength <= *v_to) &&
             from[i].two_kernel_improvement() < two_kernel_significance() )
        {
            /* Write localization behind array. Array will be enlarged
             * later. */
            new (to+to_count) Localization( from[i] );
            
            /* Move localization by drift correction. */
            Localization::Position::Type shift 
                = (to[i].frame_number() * shift_velocity).cast< Localization::Position::Type::Scalar >();
            to[i].position() += shift;
                
            const Localization::Position::Type& npos = to[i].position();
            /* Check if new coordinates are still valid. */
            if ( traits.position().is_in_range( npos ) )
            {
                /* Do not increase to_count to cause localizations to be
                 * overwritten later. */
            } else {
                to_count++;
            }
        }
    }
}

Output::Result 
LocalizationFilter::emit_localizations( 
    const Localization* p, int n, 
    frame_index forImage, const Localization *p2, int n2)
{
    data_cpp::Vector<Localization> buffer (n+n2);
    int end = 0;
    if( p != NULL) copy_and_modify_localizations(p, n, buffer.ptr(), end);
    if( p2 != NULL) copy_and_modify_localizations(p2, n2, buffer.ptr(), end);

    EngineResult eo;
    eo.number = end;
    eo.first = buffer.ptr();
    eo.forImage = forImage;
    return output->receiveLocalizations(eo);
}

Output::AdditionalData
LocalizationFilter::announceStormSize(const Announcement& a) 

{ 
    boost::units::quantity<ShiftSpeed,float> standstill
        ( 0 * boost::units::si::meters_per_second );
    traits = a;
    if ( ( ! traits.image_number().resolution().is_set() ) && (
        x_shift() != standstill || y_shift() != standstill) )
        throw std::runtime_error("Pixel size or acquisition speed is unknown, but drift correction given");
    /* TODO traits.speed */
    if ( x_shift() != standstill || y_shift() != standstill ) {
        shift_velocity.x() = 
            AppliedSpeed(x_shift() / (*traits.image_number().resolution()));
        shift_velocity.y() = 
            AppliedSpeed(y_shift() / (*traits.image_number().resolution()));
    } else {
        shift_velocity.fill( AppliedSpeed::from_value(0) );
    }
    {
        ost::MutexLock lock( locStoreMutex );
        localizationsStore.announceStormSize(a);
    }

    Announcement my_announcement(a);
    my_announcement.result_repeater = re_emitter.get();
    AdditionalData data = output->announceStormSize(my_announcement); 
    Output::check_additional_data_with_provided(
        "LocalizationFilter", AdditionalData().set_cluster_sources(), data );
    inputState = outputState = Running;
    return data;
}

void LocalizationFilter::propagate_signal(ProgressSignal s)
{
    {
        ost::MutexLock lock( locStoreMutex );
        localizationsStore.propagate_signal(s);
    }
    ost::ReadLock lock( emissionMutex );
    if ( s == Engine_run_succeeded ) 
        inputState = outputState = Succeeded;
    output->propagate_signal(s); 
}

Output::Result 
LocalizationFilter::receiveLocalizations(const EngineResult& e) 
{
    {
        ost::MutexLock lock( locStoreMutex );
        localizationsStore.receiveLocalizations( e );
    }

    Output::Result rv;
    {
        ost::ReadLock lock( emissionMutex );
        rv = emit_localizations( e.first, e.number, e.forImage );
    }
    return rv;
}

void LocalizationFilter::operator()
    (const simparm::Event& e) 
{
    if ( &e.source == &from.value ) {
        v_from = from();
        re_emitter->repeat_results();
    } else if ( &e.source == &to.value ) {
        v_to = to();
        re_emitter->repeat_results();
    } else if ( &e.source == &x_shift.value ) {
        shift_velocity.x() = AppliedSpeed(
            x_shift() / (*traits.image_number().resolution()) );
        DEBUG( "Setting X shift velocity to " << shift_velocity.x() );
        re_emitter->repeat_results();
    } else if ( &e.source == &y_shift.value ) {
        shift_velocity.y() = AppliedSpeed( y_shift() / (*traits.image_number().resolution()) );
        re_emitter->repeat_results();
    } else if ( &e.source == &two_kernel_significance.value ) {
        re_emitter->repeat_results();
    } else if ( &e.source == &shift_scale.value ) {
        x_shift.min = -1.0 * shift_scale();
        y_shift.min = -1.0 * shift_scale();
        x_shift.max = 1.0 * shift_scale();
        y_shift.max = 1.0 * shift_scale();
        x_shift.increment = 0.01 * shift_scale();
        y_shift.increment = 0.01 * shift_scale();
    }
}

LocalizationFilter::_Config::_Config()
: simparm::Object("LocalizationFilter", "Filter localizations"),
  from("MinimumAmplitude", "Minimum localization strength"),
    to("MaximumAmplitude", "Maximum localization strength"),
    shift_scale("ShiftScale", "Maximum drift correction"),
    x_shift("XDrift", "X drift correction"),
    y_shift("YDrift", "Y drift correction"),
    two_kernel_significance("TwoKernelSignificance", 
        "Double spot ratio threshold", 0.02)
{
    from.helpID = HELP_Filter_MinStrength;

    to.helpID = HELP_Filter_MaxStrength;
    to.setUserLevel(simparm::Object::Beginner);
    to.setHelp("Every fit attempt with an amplitude higher "
        "than this value will be considered an artifact and discarded "
        "from the results. This can be useful for filtering double "
        "fluorophore emissions.");

    x_shift.helpID = HELP_Filter_XDrift;
    y_shift.helpID = HELP_Filter_YDrift;
    shift_scale = boost::units::quantity<ShiftSpeed,double>(
        1E-9f * boost::units::si::meters_per_second);
    x_shift.min = -1.0 * shift_scale();
    y_shift.min = -1.0 * shift_scale();
    x_shift.max = 1.0 * shift_scale();
    y_shift.max = 1.0 * shift_scale();
    x_shift.increment = 0.01 * shift_scale();
    y_shift.increment = 0.01 * shift_scale();
    two_kernel_significance.setMin(0);
    two_kernel_significance.setMax(1);
    two_kernel_significance.setIncrement(0.001);
}

void LocalizationFilter::_Config::registerNamedEntries() {
    push_back(from);
    push_back(to);
    push_back(shift_scale);
    push_back(x_shift);
    push_back(y_shift);
    push_back(two_kernel_significance);
}

}
}
