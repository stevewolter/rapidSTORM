#include "MemoryCache.h"
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/doc/context.h>

#include "debug.h"
#include <dStorm/error_handler.h>
#include <string>

#include <dStorm/traits/range_impl.h>
#include <dStorm/output/Localizations_iterator.h>

namespace dStorm {
namespace output {

class MemoryCache::ReEmitter 
: public ost::Thread, public ResultRepeater
{
    /** Flag indicating whether reemittance is desired. This
      * flag will only be set to false in the subthread, thus
      * not needing a mutex. */
    bool reemittance_needed;
    /** Mutex for the condition. */
    ost::Mutex mutex;
    /** Condition that indicates change in parameter_changed
      * or need_re_emitter. */
    ost::Condition condition;
    /** This flag will be set to false when the re_emitter should
      * terminate for destruction of the MemoryCache. */
    bool need_re_emitter;

    MemoryCache &work_for;

  public:
    ReEmitter(MemoryCache& work_for) :
        ost::Thread("Localization re-emitter"),
        reemittance_needed(false),
        condition(mutex),
        need_re_emitter(true),
        work_for(work_for)
    {
        ost::WriteLock lock( work_for.emissionMutex );
        work_for.Filter::propagate_signal( Engine_run_is_starting );
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
        DEBUG("Acquiring mutex");
        mutex.enterMutex();
        DEBUG("Acquired mutex");
        while (need_re_emitter) {
            DEBUG("Running loop");
            if ( reemittance_needed ) {
                reemittance_needed = false;
                mutex.leaveMutex();
                try {
                    work_for.reemit_localizations( reemittance_needed );
                } catch (const std::bad_alloc& e) {
                    std::cerr << "Ran out of memory." << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "An error occured during result recomputation: "
                            << e.what() << std::endl;
                } catch (...) {
                    std::cerr << "An unknown error occured during result recomputation."
                            << std::endl;
                }
                mutex.enterMutex();
            }
            if (!reemittance_needed && need_re_emitter) {
                DEBUG("Waiting for next iteration");
                condition.wait();
                DEBUG("Waited for next iteration");
            }
        }
        mutex.leaveMutex();
      DEBUG("Finished reemitter subthread");
    }

    void repeat_results() {
        reemittance_needed = true;
        condition.signal();
    }
};

MemoryCache::MemoryCache(
    std::auto_ptr<output::Output> output
)
: Filter(output),
  re_emitter( new ReEmitter(*this) )
{ 
    re_emitter->start();
}

MemoryCache::MemoryCache(
    const MemoryCache& o
)
: Filter(o),
  re_emitter( new ReEmitter(*this) )
{ 
    re_emitter->start();
}

MemoryCache::~MemoryCache() {}

void MemoryCache::reemit_localizations(bool& terminate) {
    ost::MutexLock lock_a( locStoreMutex );
    ost::WriteLock lock_b( emissionMutex );
    if ( outputState == Running )
        Filter::propagate_signal( Output::Engine_run_is_aborted );
    if ( outputState != PreStart )
        Filter::propagate_signal( Output::Engine_is_restarted );

    typedef Localizations::image_wise_const_iterator const_iterator;
    for ( const_iterator i = store.begin_imagewise(); i != store.end_imagewise(); ++i )
    {
        Result r = Filter::receiveLocalizations( *i );
        /* TODO: Result not checked for now. */
        if ( terminate || ErrorHandler::global_termination_flag() )
            break;
    }
            
    if ( terminate || ErrorHandler::global_termination_flag() ) {
        Filter::propagate_signal( Engine_run_is_aborted );
        return;
    }

    if ( inputState == Succeeded ) {
        Filter::propagate_signal( Engine_run_succeeded );
        outputState = Succeeded;
    }
}

Output::AdditionalData
MemoryCache::announceStormSize(const Announcement& a) 
{ 
    {
        ost::MutexLock lock( locStoreMutex );
        store = Localizations(a);
    }

    Announcement my_announcement(a);
    my_announcement.result_repeater = re_emitter.get();
    AdditionalData data = Filter::announceStormSize(my_announcement); 
    Output::check_additional_data_with_provided(
        "MemoryCache", AdditionalData().set_cluster_sources(), data );
    inputState = outputState = Running;
    return data;
}

void MemoryCache::propagate_signal(ProgressSignal s)
{
    if ( s == Engine_is_restarted )
    {
        ost::MutexLock lock( locStoreMutex );
        store.clear();
    }
    ost::ReadLock lock( emissionMutex );
    if ( s == Engine_run_succeeded ) 
        inputState = outputState = Succeeded;
    Filter::propagate_signal(s); 
}

Output::Result 
MemoryCache::receiveLocalizations(const EngineResult& e) 
{
    Localizations::image_wise_iterator i;
    {
        ost::MutexLock lock( locStoreMutex );
        i = store.insert( e );
    }

    Output::Result rv;
    {
        ost::ReadLock lock( emissionMutex );
        rv = Filter::receiveLocalizations( *i );
    }
    return rv;
}

MemoryCacheSource::MemoryCacheSource()
: simparm::Object("Cache", "Cache localizations"),
  FilterSource( static_cast<simparm::Node&>(*this) ) {}

MemoryCacheSource::MemoryCacheSource(const MemoryCacheSource& o)
: simparm::Object(o), FilterSource(static_cast<simparm::Node&>(*this), o) {}

std::auto_ptr<Output> MemoryCacheSource::make_output()
{
    return std::auto_ptr<Output>( new MemoryCache( FilterSource::make_output() ) );
}

template <>
std::auto_ptr<OutputSource> make_output_source<MemoryCache>()
{
    return std::auto_ptr<OutputSource>( new MemoryCacheSource() );
}


}
}
