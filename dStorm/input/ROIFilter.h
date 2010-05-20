#ifndef DSTORM_INPUT_ROIFILTER_H
#define DSTORM_INPUT_ROIFILTER_H

namespace dStorm {
namespace input {

template <typename Ty>
class ROIFilter {
    frame_index from, to;
  public:
    ROIFilter(frame_index from, frame_index to)
        : from(from), to(to) {}
    
    typename Source<Ty>::TraitsPtr 
    operator()(typename Source<Ty>::TraitsPtr p) const
    {
        const frame_index one_frame = 1 * cs_units::camera::frame;
        frame_index to = this->to;
        if ( p->total_frame_count.is_set() ) {
            frame_index& fc = *p->total_frame_count;
            to = std::min(to, fc - one_frame);
            fc = std::min(to - from + one_frame, fc);
        }
        return p;
    }

    bool operator()(const Ty& t) {
        return t.frame_number() >= from && t.frame_number() <= to;
    }
};

}
}

#endif
