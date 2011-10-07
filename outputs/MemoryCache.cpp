#include "MemoryCache.h"
#include <dStorm/Engine.h>

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
        std::for_each( storages.begin(), storages.end(),
            boost::bind( &Part::recall, _1, offsets[index].second, into.begin(), into.end() ) );
        for_each( into.begin(), into.end(), 
            bind(&Localization::frame_number, boost::lambda::_1) = into.forImage );
    }
};

MemoryCache::MemoryCache(
    std::auto_ptr<output::Output> output
)
: Filter(output), engine_run_has_succeeded(false)
{ 
}

MemoryCache::MemoryCache( const MemoryCache& o )
: Filter(o)
{ 
    throw std::logic_error("Not implemented");
}

MemoryCache::~MemoryCache() {
    {
        boost::lock_guard<boost::mutex> lock(reemittance_mutex);
        reemit_count = -1;
        count_changed.notify_all();
    }
    reemitter.join();
}

#if 0
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
#endif

void MemoryCache::run_reemitter() {
    int last_reemit = 0;
    while ( true ) {
        DEBUG("Locking re-emittance mutex");
        boost::unique_lock<boost::mutex> lock(reemittance_mutex);
        DEBUG("Acting on " << reemit_count << " " << last_reemit);
        if ( reemit_count < last_reemit ) {
            DEBUG("Aborting");
            break;
        } else if ( reemit_count == last_reemit ) {
            DEBUG("Waiting for work");
            count_changed.wait( lock );
        } else {
            DEBUG("Re-emitting localizations");
            int my_count = ++last_reemit;
            lock.unlock();
            try {
                reemit_localizations(my_count);
            } catch (const std::bad_alloc& e) {
                std::cerr << "Ran out of memory. Your results may be incomplete." << std::endl;
            } catch (const std::runtime_error& e) {
                std::cerr << "An error occured during result recomputation: "
                        << e.what() << std::endl;
            }

            lock.lock();
            DEBUG("Re-emitted localizations");
        }
    }
}

void MemoryCache::repeat_results() {
    boost::lock_guard<boost::mutex> lock(reemittance_mutex);
    ++reemit_count;
    count_changed.notify_all();
}

void MemoryCache::reemit_localizations(const int my_count) {
    boost::unique_lock<boost::recursive_mutex> lock( *output_mutex );
    DEBUG("Got output lock");
    LocalizedImage output;
    Filter::propagate_signal( Output::Engine_run_is_aborted );
    Filter::propagate_signal( Output::Engine_is_restarted );

    for ( Bunches::iterator i = bunches.begin(); i != bunches.end(); ++i ) 
        for ( int image = 0; image < i->number_of_images(); ++image ) 
        {
            {
                boost::lock_guard<boost::mutex> guard( reemittance_mutex );
                if ( my_count < reemit_count ) return;
            }
            i->recall(image, output);
            Filter::receiveLocalizations( output );
        }
    if ( engine_run_has_succeeded )
        Filter::propagate_signal( Output::Engine_run_succeeded );
}

Output::AdditionalData
MemoryCache::announceStormSize(const Announcement& a) 
{ 
    output_mutex = a.output_chain_mutex;
    bunches.clear();
    master_bunch.reset( new Bunch(a) );
    bunches.push_back( new Bunch(*master_bunch) );

    reemit_count = 0;
    DEBUG("Making reemitter thread");
    reemitter = boost::thread( &MemoryCache::run_reemitter, this );
    DEBUG("Made reemitter thread");

    Announcement my_announcement(a);
    my_announcement.engine = this;
    AdditionalData data = Filter::announceStormSize(my_announcement); 
    Output::check_additional_data_with_provided(
        "MemoryCache", AdditionalData().set_cluster_sources(), data );
    return data;
}

void MemoryCache::propagate_signal(ProgressSignal s)
{
    if ( s == Engine_is_restarted )
    {
        engine_run_has_succeeded = false;
        bunches.clear();
        bunches.push_back( new Bunch(*master_bunch) );
    } else if ( s == Engine_run_succeeded )
        engine_run_has_succeeded = true;
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
