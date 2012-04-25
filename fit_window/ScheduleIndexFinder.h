#ifndef DSTORM_GUF_SCHEDULE_INDEX_FINDER_H
#define DSTORM_GUF_SCHEDULE_INDEX_FINDER_H

#include "Optics.h"

namespace dStorm {
namespace engine { class InputPlane; }
namespace fit_window {

/** This class selects appropriate instantiations from a compile-time list.
 */
class ScheduleIndexFinder {
    const bool do_disjoint, use_doubles;
    const Optics& optics;

    template <typename Schedule>
        class set_if_appropriate;

public:
    ScheduleIndexFinder( bool allow_disjoint_fitting, bool use_doubles, const Optics& );
    int get_evaluation_tag_index( const Spot& position ) const;

    /** Get the index of the first matching data tag in the Schedule. */
    template <typename Schedule>
    int get_evaluation_tag_index( Schedule, const Spot& position ) const;
};

}
}

#endif
