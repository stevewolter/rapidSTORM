#ifndef DSTORM_OUTPUTS_MEMORY_CACHE_H
#define DSTORM_OUTPUTS_MEMORY_CACHE_H

#include <dStorm/units/prefixes.h>
#include <dStorm/output/FilterBuilder.h>
#include <dStorm/outputs/LocalizationList.h>
#include <simparm/Structure.hh>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/UnitEntries/ADC.h>

namespace dStorm {
namespace output {

using namespace boost::units;

class MemoryCache : public Output
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
    /** Cache containing all localizations received so far. */
    outputs::LocalizationList localizationsStore;

    enum State { PreStart, Running, Succeeded };
    State inputState, outputState;

    std::auto_ptr< Output > output;

    /** Thread class that will do the actual re-emitting of localizations. */
    class ReEmitter;
    std::auto_ptr< ReEmitter > re_emitter;

    void init();

    /** This method will re-emit localizations while the flag
      * given by \c terminate is false. */
    void reemit_localizations(bool& terminate);

    class _Config;
  public:
    MemoryCache(std::auto_ptr<Output> output);
    MemoryCache(const MemoryCache&);
    ~MemoryCache();
    MemoryCache* clone() const 
        { return new MemoryCache(*this); }
    MemoryCache& operator=(const MemoryCache&);

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

}
}

#endif
