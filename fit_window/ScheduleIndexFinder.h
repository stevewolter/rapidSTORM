#ifndef DSTORM_GUF_SCHEDULE_INDEX_FINDER_H
#define DSTORM_GUF_SCHEDULE_INDEX_FINDER_H

#include "Optics.h"

namespace dStorm {
namespace engine { class InputPlane; }
namespace fit_window {

/** This class selects appropriate instantiations from a compile-time list.
 */
class ScheduleIndexFinder {
    const Optics& optics;
    std::vector<int> table;

    template <typename Schedule>
        class create_table;

public:
    template <typename Schedule>
    ScheduleIndexFinder( Schedule, bool allow_disjoint_fitting, bool use_doubles, const Optics&, int max_width );

    /** Get the index of the first matching data tag in the Schedule. */
    int get_evaluation_tag_index( const Spot& position ) const;
};

}
}

#endif
