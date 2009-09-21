#ifndef LOCPREC_REGIONSEGMENTERCONFIG_H
#define LOCPREC_REGIONSEGMENTERCONFIG_H

#include <dStorm/BasicOutputs.h>

namespace locprec {
    class Outputs : public dStorm::BasicOutputs {
      public:
        Outputs();
        virtual Outputs* clone() const { return new Outputs(); }
    };
}

#endif
