#define DSTORM_LOCALIZATIONLIST_CPP
#include "LocalizationList.h"
#include <cassert>

using namespace std;

namespace dStorm {
namespace outputs {
    output::Output::AdditionalData
    LocalizationList::announceStormSize(const Announcement &a)
    { 
        ost::MutexLock lock(mutex);
        input::Traits<Localization> traits(a.traits, a.length);
        if (target == NULL) {
            allocTarget.reset( new output::Localizations( traits ) );
            target = allocTarget.get();
        } else
            target->setDim( traits );

        return NoData;
    }

    output::Output::Result 
        LocalizationList::receiveLocalizations(const EngineResult& er)
    { 
        ost::MutexLock lock(mutex);
        assert(target != NULL);
        for (int i = 0; i < er.number; i++) 
            target->push_back( er.first[i] ); 
        return KeepRunning; 
    }

}
}
