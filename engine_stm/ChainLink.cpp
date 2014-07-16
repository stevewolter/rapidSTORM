#include "debug.h"

#include "engine_stm/ChainLink.h"
#include "engine_stm/LocalizationBuncher.h"
#include "input/MetaInfo.h"
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

class ChainLink : public input::Forwarder {
  public:
    ChainLink* clone() const OVERRIDE { return new ChainLink(*this); }
    input::BaseSource* makeSource() {
        std::auto_ptr<input::Source<localization::Record>> upstream =
            BaseSource::downcast<localization::Record>(upstream_source());
        return new Source<Type>(upstream);
    }

    void traits_changed(TraitsRef orig, Link*) {
        if (!orig) {
            update_current_meta_info(orig);
            return;
        }

        auto my_info = boost::make_shared<MetaInfo>(*orig);
        if (orig->provides<localization::Record>()) {
            my_info->set_traits(new Traits<output::LocalizedImage(
                        *orig->traits<output::LocalizedImage>(),
                        "STM", "Localizations file"));
        }
        return my_info;
    }
};

std::auto_ptr<input::Link>
make_localization_buncher()
{
    return std::auto_ptr<input::Link>( new ChainLink( ) );
}

}
}
