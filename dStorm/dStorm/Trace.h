#ifndef DSTORM_TRACE
#define DSTORM_TRACE

#include <dStorm/Localization.h>
#include <data-c++/Vector.h>

namespace dStorm {
    class Trace : public data_cpp::Vector<Localization> {
      private:
        bool computed_variance;
        double sd_x, sd_y;
        void compute_SD() throw();
      public:
        double get_X_SD() const throw() { 
            if (!computed_variance) const_cast<Trace&>(*this).compute_SD();
            return sd_x;
        }
        double get_Y_SD() const throw() { 
            if (!computed_variance) const_cast<Trace&>(*this).compute_SD();
            return sd_y;
        }

    };
}

#endif
