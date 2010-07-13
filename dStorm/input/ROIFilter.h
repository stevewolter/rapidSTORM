#ifndef DSTORM_INPUT_ROIFILTER_H
#define DSTORM_INPUT_ROIFILTER_H

#include "debug.h"
#include <simparm/optional.hh>
#include <boost/units/io.hpp>

namespace dStorm {
namespace input {

template <typename Ty>
class ROIFilter {
    frame_index from;
    simparm::optional<frame_index> to;
  public:
    ROIFilter(frame_index from, simparm::optional<frame_index> to)
        : from(from), to(to) {}
    
    typename Source<Ty>::TraitsPtr 
    operator()(typename Source<Ty>::TraitsPtr p) const
    {
        const frame_index one_frame = 1 * cs_units::camera::frame;
        frame_index from = std::max( this->from, p->first_frame );
        if ( p->last_frame.is_set() && to.is_set() ) {
            p->last_frame = std::min(*to, *p->last_frame);
        }
        p->first_frame = from;
        DEBUG("First frame is " << p->first_frame << ", last frame set is " << to.is_set());
        return p;
    }

    bool operator()(const Ty& t) {
        bool rv = t.frame_number() >= from 
            && (!to.is_set() || t.frame_number() <= *to);
        DEBUG("ROI filter returns " << rv << " for " << t.frame_number());
        return rv;
    }
};

}
}

#endif
