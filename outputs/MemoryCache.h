#ifndef DSTORM_OUTPUTS_MEMORY_CACHE_H
#define DSTORM_OUTPUTS_MEMORY_CACHE_H

#include <dStorm/output/Filter.h>
#include <dStorm/output/FilterSource.h>
#include <dStorm/output/Localizations.h>
#include <dStorm/input/LocalizationTraits.h>
#include <dStorm/Engine.h>
#include <dStorm/stack_realign.h>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

namespace dStorm {
namespace output {

using namespace boost::units;

class MemoryCache 
    : public Filter, private dStorm::Engine
{
  private:
    boost::recursive_mutex* output_mutex;
    /** Cache containing all localizations received so far. */
    struct Bunch;
    std::auto_ptr<Bunch> master_bunch;
    typedef boost::ptr_list<Bunch> Bunches;
    Bunches bunches;

    boost::thread reemitter;
    DSTORM_REALIGN_STACK void run_reemitter();
    void reemit_localizations(const int);

    boost::mutex reemittance_mutex;
    boost::condition count_changed;
    int reemit_count;
    bool engine_run_has_succeeded;

    class _Config;

    static const int LocalizationsPerBunch;

    void repeat_results();
    void restart() { throw std::logic_error("Not implemented, sorry."); }
    void stop() { throw std::logic_error("Not implemented, sorry."); }
    bool can_repeat_results() { return true; }
    void change_input_traits( std::auto_ptr< input::BaseTraits > ) { throw std::logic_error("Not implemented, sorry."); }
    std::auto_ptr<EngineBlock> block() { throw std::logic_error("Not implemented"); }

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
