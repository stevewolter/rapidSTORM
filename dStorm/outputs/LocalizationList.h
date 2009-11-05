#ifndef DSTORM_LOCALIZATIONLIST_H
#define DSTORM_LOCALIZATIONLIST_H

#include <dStorm/output/Output.h>
#include <dStorm/output/Localizations.h>
#include <memory>

namespace dStorm {
    class LocalizationList : public Output, public simparm::Object {
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
        LocalizationList( Localizations* target = NULL )
            : Object("LocalizationList", "Localization target file"),
              target(target) {}
        virtual ~LocalizationList() {}
        LocalizationList* clone() const
            { throw std::runtime_error("Not implemented."); }

        AdditionalData announceStormSize(const Announcement &a);
        Result receiveLocalizations(const EngineResult&);
        void propagate_signal(ProgressSignal s) { 
            if ( s == Engine_is_restarted ) {
                ost::MutexLock lock(mutex);
                target->clear(); 
            }
        }

        bool hasResults() const { return target != NULL; }
        const Localizations& getResults() const { return *target; }
        Localizations& getResults() { return *target; }
        bool mayReleaseResults() const 
            { return allocTarget.get() != NULL; }
        std::auto_ptr<Localizations> releaseResults()
            { return allocTarget; }
    };

}
#endif
