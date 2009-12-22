#define DSTORM_LOCALIZATIONLIST_CPP
#include "LocalizationList.h"
#include <cassert>

using namespace std;

namespace dStorm {
namespace outputs {

    LocalizationList::LocalizationList( output::Localizations* target )
        : OutputObject("LocalizationList", "Localization target file"),
            target(target) {}

    LocalizationList::LocalizationList( const LocalizationList& o )
        : OutputObject(o), mutex(), 
          allocTarget( (o.allocTarget.get()) ? 
            new output::Localizations(*o.allocTarget) : NULL ),
          target( (o.allocTarget.get()) ? allocTarget.get() : o.target )
        {}

    output::Output::AdditionalData
    LocalizationList::announceStormSize(const Announcement &a)
    { 
        ost::MutexLock lock(mutex);
        input::Traits<Localization> traits
             = a.traits.get_other_dimensionality<Localization::Dim>();

        if (target == NULL) {
            allocTarget.reset( new output::Localizations( traits ) );
            target = allocTarget.get();
        } else
            target->setDim( traits );

        return AdditionalData();
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
