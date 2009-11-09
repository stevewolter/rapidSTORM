#ifndef DSTORM_LOCALIZATION_FILTER_H
#define DSTORM_LOCALIZATION_FILTER_H

#include <dStorm/output/FilterBuilder.h>
#include <dStorm/outputs/LocalizationList.h>
#include <simparm/NumericEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/data-c++/Vector.h>

namespace dStorm {
namespace output {

/** The LocalizationFilter transmission filters localizations by amplitude and
 *  provides linear drift correction. Both corrections are performed dynamically,
 *  that is, the parameters can be changed at run-time and the localization filter
 *  will re-compute and re-publish the previous results with the updated parameters. */
class LocalizationFilter : public simparm::Object, public Output,
                           public simparm::Node::Callback
{
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

    simparm::DoubleEntry from, to;
    simparm::DoubleEntry x_shift, y_shift;
    double v_from, v_to;
    double v_x_shift, v_y_shift;
    int storm_width, storm_height, storm_length;

    enum State { PreStart, Running, Succeeded };
    State inputState, outputState;

    std::auto_ptr< Output > output;

    /** Thread that will do the actual re-emitting. */
    class ReEmitter;
    std::auto_ptr< ReEmitter > re_emitter;

    void init();

    void copy_and_modify_localizations(
        const Localization *from, int n, Localization *to, int& to_count )
;
    /** This method concatenates up to two localization arrays in a 
     *  temporary buffer and sends them to the output transmission.
     *  The call to this method must be protected by either a read- or
     *  a write-lock on emissionMutex. */
    Output::Result
        emit_localizations( const Localization* p, int n, int forImage, 
                            const Localization* p2 = NULL, int n2 = 0)
;

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

    void operator()(simparm::Node&, Cause, simparm::Node*);

    AdditionalData announceStormSize(const Announcement&) 
;

    void propagate_signal(ProgressSignal s);

    Result receiveLocalizations(const EngineResult& e);
};

class LocalizationFilter::_Config : public simparm::Object {
  protected:
    void registerNamedEntries();
  public:
    _Config();

    simparm::DoubleEntry from, to;
    simparm::DoubleEntry x_shift, y_shift;
};

}
}

#endif
