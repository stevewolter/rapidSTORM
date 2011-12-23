#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/MetaInfo.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/ImageTraits.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include <dStorm/input/Source.h>
#include <dStorm/input/MetaInfo.h>
#include <dStorm/output/LocalizedImage.h>

namespace dStorm {
namespace noop_engine {

using namespace input;

class ChainLink
: public simparm::Object,
  public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;
    typedef boost::mpl::vector<dStorm::engine::Image> SupportedTypes;

    boost::shared_ptr< Traits<output::LocalizedImage> >
    create_traits( MetaInfo& my_info,
                   const Traits<engine::Image>& orig_traits ) 
    {
        return Engine::convert_traits(orig_traits);
    }

    BaseSource* make_source( std::auto_ptr< Source<engine::Image> > p )
        { return new Engine(p); }

  public:
    ChainLink()
        : simparm::Object("Noop", "Do not localize") {}

    simparm::Node& getNode() { return *this; }
};

std::auto_ptr<input::Link>
makeLink()
{
    return std::auto_ptr<input::Link>( new ChainLink( ) );
}

}
}
