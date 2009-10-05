#define DSTORM_LOCALIZATIONLIST_CPP
#include "LocalizationList.h"
#include <cassert>

using namespace std;

namespace dStorm {
    Output::AdditionalData
    LocalizationList::announceStormSize(const Announcement &a)
    { 
        ost::MutexLock lock(mutex);
        if (target == NULL) {
            allocTarget.reset( 
                new Localizations(a.width,a.height,a.length) ); 
            target = allocTarget.get();
        } else
            target->setDim(a.width,a.height,a.length);

        return NoData;
    }

    Output::Result 
        LocalizationList::receiveLocalizations(const EngineResult& er)
    { 
        ost::MutexLock lock(mutex);
        assert(target != NULL);
        for (int i = 0; i < er.number; i++) 
            target->push_back( er.first[i] ); 
        return KeepRunning; 
    }
}
