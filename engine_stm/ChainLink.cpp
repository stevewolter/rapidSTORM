#include "debug.h"

#include <boost/make_shared.hpp>
#include <boost/mpl/vector.hpp>

#include "engine_stm/ChainLink.h"
#include "engine_stm/LocalizationBuncher.h"
#include "input/Forwarder.h"
#include "input/MetaInfo.h"
#include "input/Source.h"
#include "Localization_decl.h"
#include "localization/record.h"
#include "output/LocalizedImage_decl.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace engine_stm {

using namespace input;

class ChainLink : public input::Forwarder {
  public:
    ChainLink* clone() const OVERRIDE { return new ChainLink(*this); }
    input::BaseSource* makeSource() {
        std::auto_ptr<input::Source<localization::Record>> upstream =
            BaseSource::downcast<localization::Record>(upstream_source());
        return new Source(upstream);
    }

    void traits_changed(TraitsRef orig, Link*) {
        if (!orig) {
            update_current_meta_info(orig);
            return;
        }

        auto my_info = boost::make_shared<MetaInfo>(*orig);
        if (orig->provides<localization::Record>()) {
            my_info->set_traits(new Traits<output::LocalizedImage>(
                        *orig->traits<localization::Record>(),
                        "STM", "Localizations file"));
        }
        update_current_meta_info(my_info);
    }
};

std::auto_ptr<input::Link>
make_localization_buncher()
{
    return std::auto_ptr<input::Link>( new ChainLink( ) );
}

}
}
