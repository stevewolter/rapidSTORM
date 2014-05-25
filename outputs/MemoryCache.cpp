#include "outputs/MemoryCache.h"
#include "base/Engine.h"

#include "debug.h"
#include <string>

#include "output/Filter.h"
#include "output/FilterSource.h"
#include "output/FilterBuilder.h"
#include "localization/Traits.h"
#include "base/Engine.h"
#include "stack_realign.h"
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>

#include "stack_realign.h"
#include <boost/thread/locks.hpp>
#include "outputs/MemoryCache_Cache.h"
#include <boost/lambda/lambda.hpp>
#include <boost/lambda/bind.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>

namespace dStorm {
namespace memory_cache {

struct StoreTree {
    typedef boost::ptr_vector< Store > Stores;
    Stores parts;
    boost::optional<int> repetitions;
    typedef std::vector< StoreTree > Children;
    Children children_parts;
    int current_offset;

    StoreTree( const input::Traits<Localization>& traits ) 
        : repetitions( traits.repetitions ), current_offset(0)
    {
        parts = Store::instantiate_necessary_caches( traits );
        for ( input::Traits<Localization>::Sources::const_iterator 
            i = traits.source_traits.begin(); i != traits.source_traits.end(); ++i )
            children_parts.push_back( StoreTree( **i ) );
    }
};

class Localizations {
  protected:
    int offset, count;
    boost::optional< std::vector< Localizations > > children;
    typedef std::vector<Localization>::iterator Input;
    typedef std::vector<Localization>::const_iterator ConstInput;

  public:
    Localizations( ConstInput b, ConstInput last, StoreTree& n );
    void recall( Input begin, const StoreTree& n ) const;
};

class LocalizedImage : private Localizations {
    int group;
  public:
    LocalizedImage( const output::LocalizedImage& i, StoreTree& n )
        : Localizations( i.begin(), i.end(), n ), group(i.group) {}
    void recall( output::LocalizedImage& to, const StoreTree& n ) const {
        to.resize( count );
        Localizations::recall( to.begin(), n );
        to.group = group;
    }
};

class Bunch 
{
    StoreTree storages_root;
    std::vector< LocalizedImage > offsets;
    int localization_count;

  public:
    Bunch(const input::Traits<output::LocalizedImage>& traits) 
        : storages_root( traits), localization_count(0) {}
    ~Bunch() {}

    void insert( const output::LocalizedImage& o );
    int number_of_images() const { return offsets.size(); }
    int number_of_localizations() const { return localization_count; }
    void recall( int index, output::LocalizedImage& into ) const; 
};

struct Config 
{
    static std::string get_name() { return "Cache"; }
    static std::string get_description() { return "Cache localizations"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }
    void attach_ui( simparm::NodeHandle ) {}
    bool determine_output_capabilities( dStorm::output::Capabilities& cap ) { 
        cap.set_source_image( false );
        return true;
    }
};

class Output 
    : public output::Filter, private dStorm::Engine
{
  private:
    boost::mutex output_mutex;
    std::auto_ptr<Bunch> master_bunch;
    typedef boost::ptr_list<Bunch> Bunches;
    Bunches bunches;

    boost::thread reemitter;
    boost::recursive_mutex suboutputs;
    DSTORM_REALIGN_STACK void run_reemitter();
    void reemit_localizations(const int);

    boost::mutex reemittance_mutex;
    boost::condition count_changed;
    int reemit_count;
    boost::optional<bool> engine_run_has_succeeded;

    Engine* upstream;

    class _Config;

    static const int LocalizationsPerBunch = 100000;

    void repeat_results();
    void restart() { throw std::logic_error("Not implemented, sorry."); }
    void stop() { throw std::logic_error("Not implemented, sorry."); }
    bool can_repeat_results() { return true; }
    void change_input_traits( std::auto_ptr< input::BaseTraits > ) { throw std::logic_error("Not implemented, sorry."); }
    std::auto_ptr<EngineBlock> block() 
        { return upstream->block(); }
    std::auto_ptr<EngineBlock> block_termination() 
        { return upstream->block_termination(); }

    void prepare_destruction_();
    void store_results_( bool success );

  public:
    Output(const Config&, std::auto_ptr<output::Output> output);
    ~Output();

    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement& r) ;
    void receiveLocalizations(const EngineResult& e);

};


Localizations::Localizations( ConstInput b, ConstInput last, StoreTree& n )
: offset( n.current_offset ), count( (n.repetitions.is_initialized()) ? *n.repetitions : (last-b) )
{
    ConstInput e = b + count;
    for ( StoreTree::Stores::iterator i = n.parts.begin(); i != n.parts.end(); ++i )
        i->store( b, e );
    n.current_offset += count;

    if ( n.children_parts.size() > 0 ) {
        children = std::vector<Localizations>();
        children->reserve( n.children_parts.size() * (e-b) );
        for ( ConstInput i = b; i != e; ++i ) {
            Localization::Children::const_iterator c = i->children->begin(), ce = i->children->end();
            StoreTree::Children::iterator part = n.children_parts.begin();
            for ( ; part != n.children_parts.end(); ++part ) {
                children->push_back( Localizations( c, ce, *part ) );
                c += children->back().count;
            }
        }
    }
}
    
void Localizations::recall( Input begin, const StoreTree& n ) const {
    std::for_each( n.parts.begin(), n.parts.end(),
        boost::bind( &Store::recall, _1, offset, begin, begin+count ) );

    if ( ! n.children_parts.empty() ) {
        std::vector< Localizations >::const_iterator offset;
        offset = children->begin();
        for ( Input l = begin; l != begin+count; ++l ) {
            int children_count = 0;
            for ( std::vector< Localizations >::const_iterator i = offset; i != offset + n.children_parts.size(); ++i )
                children_count += i->count;
            l->children = std::vector<Localization>( children_count );
            std::vector<Localization>::iterator current = l->children->begin();
            for (StoreTree::Children::const_iterator i = n.children_parts.begin(); i != n.children_parts.end(); ++i) {
                offset->recall( current, *i );
                current += offset->count;
                ++offset;
            }
        }
    }
}

void Bunch::recall( int index, output::LocalizedImage& into ) const {
    assert( index < number_of_images() );
    offsets[index].recall( into, storages_root );
}

void Bunch::insert( const output::LocalizedImage& o ) {
    offsets.push_back( LocalizedImage( o, storages_root ) );
    localization_count += o.size();
}

Output::Output(
    const Config&, std::auto_ptr<output::Output> output
)
: Filter(output)
{ 
}

void Output::prepare_destruction_() {
    {
        boost::lock_guard<boost::mutex> lock(reemittance_mutex);
        reemit_count = -1;
        count_changed.notify_all();
    }
    reemitter.join();
}

Output::~Output() {
    prepare_destruction_();
    destroy_suboutput();
}

void Output::run_reemitter() {
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

void Output::repeat_results() {
    boost::lock_guard<boost::mutex> lock(reemittance_mutex);
    ++reemit_count;
    count_changed.notify_all();
}

void Output::reemit_localizations(const int my_count) {
    boost::unique_lock<boost::mutex> lock( output_mutex );
    DEBUG("Got output lock");
    output::LocalizedImage output;
    {
        boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
        Filter::announce_run( RunAnnouncement() );
    }

    for ( Bunches::iterator i = bunches.begin(); i != bunches.end(); ++i ) 
        for ( int image = 0; image < i->number_of_images(); ++image ) 
        {
            {
                boost::lock_guard<boost::mutex> guard( reemittance_mutex );
                if ( my_count < reemit_count ) return;
            }
            i->recall(image, output);
            boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
            Filter::receiveLocalizations( output );
        }
    if ( engine_run_has_succeeded ) {
        boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
        Filter::store_children_results( *engine_run_has_succeeded );
    }
}

Output::AdditionalData
Output::announceStormSize(const Announcement& a) 
{ 
    boost::lock_guard<boost::mutex> lock(output_mutex);
    upstream = a.engine;
    master_bunch.reset( new Bunch(a) );

    reemit_count = 0;
    DEBUG("Making reemitter thread");
    reemitter = boost::thread( &Output::run_reemitter, this );
    DEBUG("Made reemitter thread");

    Announcement my_announcement(a);
    my_announcement.engine = this;
    boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
    AdditionalData data = Filter::announceStormSize(my_announcement); 
    Output::check_additional_data_with_provided(
        "MemoryCache", AdditionalData().set_cluster_sources(), data );
    return data;
}

Output::RunRequirements Output::announce_run(const RunAnnouncement& r) {
    boost::lock_guard<boost::mutex> lock(output_mutex);
    engine_run_has_succeeded.reset();
    bunches.clear();
    bunches.push_back( new Bunch(*master_bunch) );
    return Filter::announce_run(r);
}

void Output::store_results_( bool success )
{
    engine_run_has_succeeded = success;
    boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
    Filter::store_children_results( *engine_run_has_succeeded ); 
}

void Output::receiveLocalizations(const EngineResult& e) 
{
    boost::lock_guard<boost::mutex> lock(output_mutex);
    bunches.back().insert( e );
    if ( bunches.back().number_of_localizations() >= LocalizationsPerBunch )
        bunches.push_back( new Bunch( *master_bunch ) );

    boost::lock_guard<boost::recursive_mutex> suboutput_lock( suboutputs );
    Filter::receiveLocalizations( e );
}

std::auto_ptr<output::OutputSource> make_output_source()
{
    return std::auto_ptr<output::OutputSource>( new output::FilterBuilder<Config,Output>() );
}


}
}
