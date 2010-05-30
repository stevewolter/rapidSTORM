#ifndef DSTORM_PUBLISHER_H
#define DSTORM_PUBLISHER_H

namespace dStorm {
namespace viewer {

/** The Publisher class stores a pointer to the currently
    *  set listener, if any is provided. */
template <typename Listener>
struct Publisher
{
    Listener *fwd;
  public:
    inline void setListener(Listener* target)
        { fwd = target; }
    inline Listener& publish() { return *fwd; }
};

}
}

#endif
