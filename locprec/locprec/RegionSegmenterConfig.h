#ifndef LOCPREC_REGIONSEGMENTERCONFIG_H
#define LOCPREC_REGIONSEGMENTERCONFIG_H

#include <dStorm/BasicTransmissions.h>

namespace locprec {
    class LocprecTransmissionSourceFactory : public dStorm::BasicTransmissions {
      public:
        virtual std::auto_ptr<dStorm::TransmissionSource> makeTransmissionSource
            (std::string wish) throw();
    };
}

#endif
