#ifndef DSTORM_LOCALIZATIONCOUNTER_H
#define DSTORM_LOCALIZATIONCOUNTER_H

#include <dStorm/Transmission.h>
#include <iostream>
#include <memory>

namespace dStorm {
    class LocalizationCounter : public Transmission {
      private:
        ost::Mutex mutex;
      	int count;
        ConfigEntryUnsignedLong *update;
        std::auto_ptr<ConfigEntryUnsignedLong> updateStore;
        std::ostream* printAtStop;

      public:
        LocalizationCounter() throw()
            : ConfigObject("LocCountStat", "Localization counting status"),
              count(0),
              update(NULL), printAtStop(NULL) {}
        ~LocalizationCounter() throw() {}

        AdditionalData announceStormSize(int, int, int) throw() {
            ost::MutexLock lock(mutex);
            if ( this->isActive() ) {
                updateStore.reset(new ConfigEntryUnsignedLong
                    ("LocalizationCount",
                        "Number of localizations found"));
                update = updateStore.get();
                update->setIncrement(100);
                update->setUserLevel(ConfigEntry::Beginner);
                register_entry(update);
            }

            count = 0; 
            return NoData;
        }
        Result receiveLocalizations(const EngineResult& er) throw() {
            ost::MutexLock lock(mutex);
            count += er.number; 
            if (update) *update = count;
            return KeepRunning; 
        }
        void deleteAllResults() throw() {
            ost::MutexLock lock(mutex);
            count = 0; if (update) *update = 0; 
        }
        void finish() throw() { 
            ost::MutexLock lock(mutex);
            if (update) *update = count;
            if (!this->isActive()) std::cout << count << "\n"; 
        }
    };
}
#endif

