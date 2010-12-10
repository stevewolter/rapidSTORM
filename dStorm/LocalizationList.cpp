#define DSTORM_LOCALIZATIONLIST_CPP
#include <dStorm/LocalizationList.h>
#include <cassert>

using namespace std;

namespace dStorm {
    Transmission::AdditionalData
    LocalizationList::announceStormSize(int w, int h, int n) throw()
    { 
        ost::MutexLock lock(mutex);
        if (target == NULL) {
            allocTarget.reset( new Localizations(w,h,n) ); 
            target = allocTarget.get();
        } else
            target->setDim(w,h,n);

        return NoData;
    }

    Transmission::Result 
        LocalizationList::receiveLocalizations(const EngineResult& er) throw()
    { 
        ost::MutexLock lock(mutex);
        assert(target != NULL);
        for (int i = 0; i < er.number; i++) 
            target->push_back( er.first[i] ); 
        return KeepRunning; 
    }
}
