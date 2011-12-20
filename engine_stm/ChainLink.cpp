#include "debug.h"

#include "ChainLink.h"
#include "LocalizationBuncher.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/output/LocalizedImage_traits.h>
#include <dStorm/ImageTraits.h>

#include <boost/mpl/vector.hpp>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/localization/record.h>
#include <dStorm/Localization_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/output/LocalizedImage.h>
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace engine_stm {

using namespace input;

class ChainLink : public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;
    Config config;
    Config& get_config() { return config; }

    typedef boost::mpl::vector<output::LocalizedImage,localization::Record,dStorm::Localization>
        SupportedTypes;
    template <typename Type>
    bool changes_traits( const chain::MetaInfo&, const Traits<Type>& )
        { return true; }
    template <typename Type>
    BaseTraits* create_traits( chain::MetaInfo&, const Traits<output::LocalizedImage>& p ) 
        { return new Traits<output::LocalizedImage>(p); }
    template <typename Type>
    BaseTraits* create_traits( chain::MetaInfo&, const Traits<Type>& p ) 
        { return new Traits<output::LocalizedImage>(p, "STM", "Localizations file"); }

    input::BaseSource* make_source( std::auto_ptr< input::Source<output::LocalizedImage> > p ) 
        { return p.release(); }
    template <typename Type>
    input::BaseSource* make_source( std::auto_ptr< input::Source<Type> > p ) 
        { return new Source<Type>( config, p ); }

  public:
    simparm::Node& getNode() { return config; }
};

std::auto_ptr<input::chain::Link>
make_STM_engine_link()
{
    return std::auto_ptr<input::chain::Link>( new ChainLink( ) );
}

}
}
