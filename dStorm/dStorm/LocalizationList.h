#ifndef DSTORM_LOCALIZATIONLIST_H
#define DSTORM_LOCALIZATIONLIST_H

#include <dStorm/Transmission.h>
#include <dStorm/Localizations.h>
#include <memory>

namespace dStorm {
    class LocalizationList : public Transmission {
      private:
        ost::Mutex mutex;
        std::auto_ptr<Localizations> allocTarget;
        Localizations* target;

      public:
        /** Constructor.
         *  \param target An optional target fit list. When not NULL,
         *                this parameters gives the target fit list.
         *                When NULL, the LocalizationList will allocate
         *                an internal list to store the results. */
        LocalizationList( Localizations* target = NULL ) throw()
            : ConfigObject("LocalizationList", "Localization target file"),
              target(target) {}
        virtual ~LocalizationList() throw() {}

        AdditionalData announceStormSize(int w, int h, int n) throw();
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw() { 
            ost::MutexLock lock(mutex);
            target->clear(); 
        }
        void finish() throw() {}

        bool hasResults() const throw() { return target != NULL; }
        const Localizations& getResults() const throw() { return *target; }
        Localizations& getResults() throw() { return *target; }
        bool mayReleaseResults() const throw() 
            { return allocTarget.get() != NULL; }
        std::auto_ptr<Localizations> releaseResults() throw()
            { return allocTarget; }
    };

}
#endif
