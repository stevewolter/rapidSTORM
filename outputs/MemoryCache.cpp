#include "MemoryCache.h"
#include <dStorm/Engine.h>
#include <dStorm/doc/context.h>

#include "debug.h"
#include <string>

#include <dStorm/stack_realign.h>
#include <dStorm/traits/range_impl.h>
#include <dStorm/output/Localizations_iterator.h>
#include <boost/thread/locks.hpp>
#include "MemoryCache_Cache.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

namespace dStorm {
namespace output {

const int MemoryCache::LocalizationsPerBunch = 4000;

class MemoryCache::Bunch 
{
    typedef memory_cache::Interface Part;
    int current_offset;
    std::vector< std::pair<frame_index,int> > offsets;
    typedef boost::ptr_vector< Part > Parts;
    Parts storages;
    
  public:
    Bunch(const input::Traits<LocalizedImage>& traits)
        : current_offset(0), 
          storages( Part::instantiate_necessary_caches(traits) ) {}
    ~Bunch() {}

    void insert( const LocalizedImage& o ) {
        int my_offset = current_offset;
        offsets.push_back( std::make_pair( o.forImage, my_offset ) );
        std::for_each( storages.begin(), storages.end(),
            boost::bind( &Part::store, _1, o.begin(), o.end() ) );
        current_offset += o.size();
    }

    int number_of_images() const { return offsets.size(); }
    int number_of_localizations() const { return current_offset; }

    void recall( int index, LocalizedImage& into ) const {
        assert( index < number_of_images() );
        into.clear();
        int next_offset = (index+1 == int(offsets.size())) ? current_offset 
                                                      : offsets[index+1].second;
        into.resize( next_offset - offsets[index].second );
        into.forImage = offsets[index].first;
        for_each( into.begin(), into.end(), 
            bind(&Localization::frame_number, boost::lambda::_1) = into.forImage );
    }
};

class MemoryCache::ReEmitter 
: public dStorm::Engine
{
    /** Flag indicating whether reemittance is desired. This
      * flag will only be set to false in the subthread, thus
      * not needing a mutex. */
    bool reemittance_needed;
    /** Mutex for the condition. */
    boost::mutex mutex;
    /** Condition that indicates change in parameter_changed
      * or need_re_emitter. */
    boost::condition condition;
    /** This flag will be set to false when the re_emitter should
      * terminate for destruction of the MemoryCache. */
    bool need_re_emitter;

    MemoryCache &work_for;
    boost::thread emitter;

  public:
    ReEmitter(MemoryCache& work_for) :
        reemittance_needed(false),
        need_re_emitter(true),
        work_for(work_for)
    {
        work_for.Filter::propagate_signal( Engine_run_is_starting );
        emitter = boost::thread( &MemoryCache::ReEmitter::run, this );
    }

    ~ReEmitter() { 
        DEBUG("Destructing Reemitter");
        need_re_emitter = false; 
        condition.notify_all();
        DEBUG("Joining subthread");
        emitter.join(); 
        DEBUG("Joined subthread");
    }

    DSTORM_REALIGN_STACK void run() 
    {
        boost::unique_lock< boost::mutex > lock( mutex );
        while (need_re_emitter) {
            if ( reemittance_needed ) {
                reemittance_needed = false;
                lock.unlock();
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
                lock.lock();
            }
            if (!reemittance_needed && need_re_emitter) {
                DEBUG("Waiting for next iteration");
                condition.wait(lock);
                DEBUG("Waited for next iteration");
            }
        }
      DEBUG("Finished reemitter subthread");
    }

    void repeat_results() {
        reemittance_needed = true;
        condition.notify_one();
    }

    void restart() { throw std::logic_error("Not implemented, sorry."); }
    void stop() { throw std::logic_error("Not implemented, sorry."); }
    bool can_repeat_results() { return true; }
    void change_input_traits( std::auto_ptr< input::BaseTraits > ) { throw std::logic_error("Not implemented, sorry."); }
};

MemoryCache::MemoryCache(
    std::auto_ptr<output::Output> output
)
: Filter(output),
  re_emitter( new ReEmitter(*this) )
{ 
}

MemoryCache::MemoryCache(
    const MemoryCache& o
)
: Filter(o),
  re_emitter( new ReEmitter(*this) )
{ 
}

MemoryCache::~MemoryCache() {}

void MemoryCache::reemit_localizations(bool& terminate) {
    boost::lock_guard<boost::recursive_mutex> lock( *mutex );
    if ( outputState == Running )
        Filter::propagate_signal( Output::Engine_run_is_aborted );
    if ( outputState != PreStart )
        Filter::propagate_signal( Output::Engine_is_restarted );

    LocalizedImage im;
    for ( Bunches::const_iterator i = bunches.begin(); i != bunches.end(); ++i )
    {
        for (int j = 0; j < i->number_of_images(); ++j) {
            i->recall( j, im );
            DEBUG("Reemitting " << im.size() << " localizations for image " << j << " in current bunch");
            Filter::receiveLocalizations( im );
            /* TODO: Result not checked for now. */
            if ( terminate )
                break;
        }
    }
            
    if ( terminate ) {
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
    mutex = a.output_chain_mutex;
    bunches.clear();
    master_bunch.reset( new Bunch(a) );
    bunches.push_back( new Bunch(*master_bunch) );

    Announcement my_announcement(a);
    my_announcement.engine = re_emitter.get();
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
        bunches.clear();
        bunches.push_back( new Bunch(*master_bunch) );
    }
    if ( s == Engine_run_succeeded ) 
        inputState = outputState = Succeeded;
    Filter::propagate_signal(s); 
}

Output::Result 
MemoryCache::receiveLocalizations(const EngineResult& e) 
{
    bunches.back().insert( e );
    if ( bunches.back().number_of_localizations() >= LocalizationsPerBunch )
        bunches.push_back( new Bunch( *master_bunch ) );

    return Filter::receiveLocalizations( e );
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
