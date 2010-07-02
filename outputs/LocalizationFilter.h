#ifndef DSTORM_LOCALIZATION_FILTER_H
#define DSTORM_LOCALIZATION_FILTER_H

#include <dStorm/units/prefixes.h>
#include <boost/units/systems/si/velocity.hpp>
#include <cs_units/camera/velocity.hpp>
#include <dStorm/units/amplitude.h>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/outputs/LocalizationList.h>
#include <simparm/NumericEntry.hh>
#include <simparm/Structure.hh>
#include <simparm/OptionalEntry.hh>
#include <dStorm/data-c++/Vector.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/UnitEntries/ADC.h>

namespace dStorm {
namespace output {

/** The LocalizationFilter transmission filters localizations by amplitude and
 *  provides linear drift correction. Both corrections are performed dynamically,
 *  that is, the parameters can be changed at run-time and the localization filter
 *  will re-compute and re-publish the previous results with the updated parameters. */
class LocalizationFilter : public OutputObject,
                           public simparm::Node::Callback
{
  public:
    typedef boost::units::si::pico_scale<boost::units::si::velocity>::type
        ShiftSpeed;
    typedef boost::units::quantity<cs_units::camera::velocity,float>
        AppliedSpeed;

  private:
    /** Mutex for localizationsStore list. */
    ost::Mutex locStoreMutex;
    /** This thread lock controls the emittance of localizations to the
     *  output transmission. Normal, parallel behaviour acquires read
     *  locks on the emissionMutex, since multiple parallel invocations
     *  of receiveLocalizations are allowed. When re-emittance of results
     *  becomes necessary, a write lock is acquired to ensure no concurrent
     *  thread emits other results. */
    ost::ThreadLock emissionMutex;
    /** List of all received localizations. Necessary if
    *   amplitude threshold is changed. */
    outputs::LocalizationList localizationsStore;

    simparm::OptionalEntry< ADCEntry::value_type > from, to;
    simparm::UnitEntry<ShiftSpeed,double> shift_scale, x_shift, y_shift;
    simparm::DoubleEntry two_kernel_significance;
    simparm::optional<amplitude> v_from, v_to;
    Eigen::Matrix<AppliedSpeed, Localization::Dim, 1>
        shift_velocity;
    output::Traits traits;

    enum State { PreStart, Running, Succeeded };
    State inputState, outputState;

    std::auto_ptr< Output > output;

    /** Thread that will do the actual re-emitting. */
    class ReEmitter;
    std::auto_ptr< ReEmitter > re_emitter;

    void init();

    void copy_and_modify_localizations(
        const Localization *from, int n, Localization *to, int& to_count );
    /** This method concatenates up to two localization arrays in a 
     *  temporary buffer and sends them to the output transmission.
     *  The call to this method must be protected by either a read- or
     *  a write-lock on emissionMutex. */
    Output::Result
        emit_localizations( const Localization* p, int n, 
                            frame_index forImage, 
                            const Localization* p2 = NULL, int n2 = 0);

    /** This method will re-emit localizations while the flag
      * given by \c terminate is false. */
    void reemit_localizations(bool& terminate);

    class _Config;
  public:
    typedef simparm::Structure<_Config> Config;
    typedef FilterBuilder<LocalizationFilter> Source;

    LocalizationFilter(const Config& config,
                     std::auto_ptr<Output> output);
    LocalizationFilter(const LocalizationFilter&);
    ~LocalizationFilter();
    LocalizationFilter* clone() const 
        { return new LocalizationFilter(*this); }
    LocalizationFilter& operator=(const LocalizationFilter&);

    void operator()(const simparm::Event&);

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement& a) 
        { return output->announce_run(a); }

    void propagate_signal(ProgressSignal s);

    Result receiveLocalizations(const EngineResult& e);

    void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames) 
        { output->check_for_duplicate_filenames(present_filenames); }
};

class LocalizationFilter::_Config : public simparm::Object {
  protected:
    void registerNamedEntries();
  public:
    _Config();

    simparm::OptionalEntry< ADCEntry::value_type > from, to;
    simparm::UnitEntry<ShiftSpeed,double> shift_scale, x_shift, y_shift;

    /** Minimum quotient between residues of two- and one-kernel
        *  model fits that must be reached so that the two-kernel fit
        *  is not considered better. */
    simparm::DoubleEntry two_kernel_significance;

    bool determine_output_capabilities( Capabilities& cap ) { 
        cap.set_intransparency_for_source_data();
        return true; 
    }
};

}
}

#endif
