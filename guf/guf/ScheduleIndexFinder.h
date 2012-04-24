#ifndef DSTORM_GUF_SCHEDULE_INDEX_FINDER_H
#define DSTORM_GUF_SCHEDULE_INDEX_FINDER_H

#include "Optics.h"
#include "Config.h"

namespace dStorm {
namespace engine { class InputPlane; }
namespace guf {

class ScheduleIndexFinder {
    const bool do_disjoint, use_floats;
    const Optics& optics;

    template <typename Schedule>
        class set_if_appropriate;

public:
    ScheduleIndexFinder( bool allow_disjoint_fitting, bool use_floats, const Optics& );
    int get_evaluation_tag_index( const guf::Spot& position ) const;

    template <typename Schedule>
    int get_evaluation_tag_index( Schedule, const guf::Spot& position ) const;
};

}
}

#endif
