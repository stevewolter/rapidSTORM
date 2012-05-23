#ifndef DSTORM_LOCALIZATIONLIST_H
#define DSTORM_LOCALIZATIONLIST_H

#include "../output/Output.h"
#include "../output/Localizations.h"
#include <memory>

namespace dStorm {
namespace outputs {
    class LocalizationList : public output::Output
    {
      private:
        std::auto_ptr<output::Localizations> allocTarget;
        output::Localizations* target;

      public:
        /** Constructor.
         *  \param target An optional target fit list. When not NULL,
         *                this parameters gives the target fit list.
         *                When NULL, the LocalizationList will allocate
         *                an internal list to store the results. */
        LocalizationList( output::Localizations* target = NULL );
        virtual ~LocalizationList() {}

        AdditionalData announceStormSize(const Announcement &a);
        void receiveLocalizations(const EngineResult&);
        RunRequirements announce_run(const RunAnnouncement&) 
            { target->clear(); return RunRequirements(); }

        bool hasResults() const { return target != NULL; }
        const output::Localizations& getResults() const { return *target; }
        output::Localizations& getResults() { return *target; }
        bool mayReleaseResults() const 
            { return allocTarget.get() != NULL; }
        std::auto_ptr<output::Localizations> releaseResults()
            { return allocTarget; }
        void attach_ui( simparm::NodeHandle ) {}
    };

}
}
#endif
