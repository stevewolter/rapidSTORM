#ifndef DSTORM_OUTPUTS_MEMORY_CACHE_H
#define DSTORM_OUTPUTS_MEMORY_CACHE_H

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/input/LocalizationTraits.h>
#include <boost/ptr_container/ptr_list.hpp>

namespace dStorm {
namespace output {

using namespace boost::units;

class MemoryCache : public Filter
{
  private:
    boost::recursive_mutex* mutex;
    /** Cache containing all localizations received so far. */
    struct Bunch;
    std::auto_ptr<Bunch> master_bunch;
    typedef boost::ptr_list<Bunch> Bunches;
    Bunches bunches;

    enum State { PreStart, Running, Succeeded };
    State inputState, outputState;

    /** Thread class that will do the actual re-emitting of localizations. */
    class ReEmitter;
    std::auto_ptr< ReEmitter > re_emitter;

    /** This method will re-emit localizations while the flag
      * given by \c terminate is false. */
    void reemit_localizations(bool& terminate);

    class _Config;

    static const int LocalizationsPerBunch;
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
