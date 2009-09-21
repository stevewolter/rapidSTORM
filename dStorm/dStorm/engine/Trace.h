#ifndef DSTORM_TRACE
#define DSTORM_TRACE

#include <dStorm/engine/Localization.h>
#include <data-c++/Vector.h>

namespace dStorm {
    /** A Trace is a representation of a continuous sequence of 
     *  localizations that are thought to belong to the same fluorophore.
     *  It has basic container semantics and can compute the standard
     *  deviations (SD) of its members. */
    class Trace : private data_cpp::Vector<Localization> {
      private:
        bool computed_variance;
        double sd_x, sd_y;
        void compute_SD();
        typedef data_cpp::Vector<Localization> Base;

      public:
        Trace() : computed_variance(false) {}
        double get_X_SD() const { 
            if (!computed_variance) const_cast<Trace&>(*this).compute_SD();
            return sd_x;
        }
        double get_Y_SD() const { 
            if (!computed_variance) const_cast<Trace&>(*this).compute_SD();
            return sd_y;
        }

        void push_back( const Trace& lv ) {
            computed_variance = false;
            data_cpp::Vector<Localization>::push_back(lv);
        }

        void push_back( const Localization& l ) {
            computed_variance = false;
            data_cpp::Vector<Localization>::push_back(l);
        }

        typedef Base::const_iterator const_iterator;

        const_iterator begin() const { return Base::begin(); }
        const_iterator end() const { return Base::end(); }

        const Localization& front() const { return Base::front(); }
        const Localization& back() const { return Base::back(); }

        void clear() { Base::clear(); computed_variance = false; }
        int size() const { return Base::size(); }

        const Base& points() const { return *this; }

    };
}

#endif
