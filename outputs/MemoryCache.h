#ifndef DSTORM_OUTPUTS_MEMORY_CACHE_H
#define DSTORM_OUTPUTS_MEMORY_CACHE_H

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/input/LocalizationTraits.h>

namespace dStorm {
namespace output {

using namespace boost::units;

class MemoryCache : public Filter
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
    Localizations store;

    enum State { PreStart, Running, Succeeded };
    State inputState, outputState;

    /** Thread class that will do the actual re-emitting of localizations. */
    class ReEmitter;
    std::auto_ptr< ReEmitter > re_emitter;

    /** This method will re-emit localizations while the flag
      * given by \c terminate is false. */
    void reemit_localizations(bool& terminate);

    class _Config;
  public:
    MemoryCache(std::auto_ptr<Output> output);
    MemoryCache( const MemoryCache& );
    ~MemoryCache();
    MemoryCache* clone() const 
        { return new MemoryCache(*this); }

    AdditionalData announceStormSize(const Announcement&);
    void propagate_signal(ProgressSignal s);
    Result receiveLocalizations(const EngineResult& e);

};

struct MemoryCacheSource
    : public simparm::Object, public FilterSource
{
    MemoryCacheSource();
    MemoryCacheSource(const MemoryCacheSource&);
    MemoryCacheSource* clone() const
        { return new MemoryCacheSource(*this); }
    virtual std::string getDesc() const { return Object::desc(); }
    std::auto_ptr<Output> make_output(); 
};

}
}

#endif
