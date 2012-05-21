#define DSTORM_LOCALIZATIONLIST_CPP
#include "LocalizationList.h"
#include <cassert>

using namespace std;

namespace dStorm {
namespace outputs {

LocalizationList::LocalizationList( output::Localizations* target )
    : target(target) {}

output::Output::AdditionalData
LocalizationList::announceStormSize(const Announcement &a)
{ 
    input::Traits<Localization> traits = a;

    if (target == NULL) {
        allocTarget.reset( new output::Localizations( traits ) );
        target = allocTarget.get();
    } else
        target->setDim( traits );

    return AdditionalData();
}

void LocalizationList::receiveLocalizations(const EngineResult& er)
{ 
    assert(target != NULL);
    target->insert( er ); 
}

}
}
