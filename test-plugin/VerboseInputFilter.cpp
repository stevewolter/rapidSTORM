#define BOOST_DISABLE_ASSERTS
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp> 
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/Image.h>
#include <iostream>

#undef DEBUG
#define VERBOSE
#include <dStorm/debug.h>
#include "VerboseInputFilter.h"

namespace VerboseInputFilter {

using namespace dStorm::input;

class ChainLink 
: public dStorm::input::Method< ChainLink >
{
    friend class dStorm::input::Method< ChainLink >;

    template <typename Type>
    bool changes_traits( const chain::MetaInfo&, const Traits<Type>& )
        { return false; }
    template <typename Type>
    void notice_traits( const chain::MetaInfo& ref, const Traits<Type>& ) {
        if ( config.verbose() )
            DEBUG("Traits " << &ref << " are passing on " << getNode().getName() << " (" << this << ")");
    }
    template <typename Type>
    BaseSource* make_source( std::auto_ptr< dStorm::input::Source<Type> > p ) {
        DEBUG( "Source of type " << typeid(*p.get()).name() << " is passing" );
        return new VerboseInputFilter::Source<Type>(config,p);
    }

  public:
    simparm::Structure<Config> config;

    simparm::Node& getNode() { return static_cast<Config&>(config); }
};

}

std::auto_ptr<dStorm::input::chain::Link>
make_verbose_input_filter() {
    return std::auto_ptr<dStorm::input::chain::Link>
        (new VerboseInputFilter::ChainLink());
}
