#ifndef BASIC_TRANSMISSIONS_H
#define BASIC_TRANSMISSIONS_H

#include <dStorm/TransmissionConfig.h>

namespace dStorm {
    class BasicTransmissions : public TransmissionSourceFactory {
      public:
        virtual std::auto_ptr<TransmissionSource> makeTransmissionSource
            (std::string wish) throw();
    };
}

#endif
