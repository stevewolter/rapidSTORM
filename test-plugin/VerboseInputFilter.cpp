#define BOOST_DISABLE_ASSERTS
#include <boost/variant.hpp>
#include <boost/serialization/variant.hpp> 
#include "VerboseInputFilter.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/Image.h>
#include <iostream>

#undef DEBUG
#define VERBOSE
#include <dStorm/debug.h>

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

  public:
    simparm::Structure<Config> config;
    BaseSource* makeSource() {
        std::auto_ptr<BaseSource> rv( Forwarder::makeSource() );
        DEBUG( "Source of type " << typeid(*rv.get()).name() << " is passing" );
        return rv.release();
    }

    simparm::Node& getNode() { return static_cast<Config&>(config); }
};

}

std::auto_ptr<dStorm::input::chain::Link>
make_verbose_input_filter() {
    return std::auto_ptr<dStorm::input::chain::Link>
        (new VerboseInputFilter::ChainLink());
}
