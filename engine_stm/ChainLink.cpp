#include "debug.h"

#include "engine_stm/ChainLink.h"
#include "engine_stm/LocalizationBuncher.h"
#include "input/MetaInfo.h"
#include "input/Method.hpp"
#include "output/LocalizedImage_traits.h"

#include <boost/mpl/vector.hpp>
#include "output/LocalizedImage_decl.h"
#include "localization/record.h"
#include "Localization_decl.h"
#include "input/Source.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace engine_stm {

using namespace input;

class ChainLink : public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;
    void attach_ui( simparm::NodeHandle at ) {}
    static std::string getName() { throw std::logic_error("Not implemented"); }

    typedef boost::mpl::vector<localization::Record,dStorm::Localization>
        SupportedTypes;
    template <typename Type>
    bool changes_traits( const MetaInfo&, const Traits<Type>& )
        { return true; }
    template <typename Type>
    BaseTraits* create_traits( MetaInfo&, const Traits<Type>& p ) 
        { return new Traits<output::LocalizedImage>(p, "STM", "Localizations file"); }

    template <typename Type>
    input::BaseSource* make_source( std::auto_ptr< input::Source<Type> > p ) 
        { return new Source<Type>( p ); }

  public:
    std::string name() const { return Forwarder::name(); }
};

std::auto_ptr<input::Link>
make_localization_buncher()
{
    return std::auto_ptr<input::Link>( new ChainLink( ) );
}

}
}
