#ifndef DSTORM_GUF_KERNELCREATOR_H
#define DSTORM_GUF_KERNELCREATOR_H

#include "Spot.h"

namespace dStorm {
namespace guf {

class MultiKernelModel;
class MultiKernelModelStack;

/** Function object to add an additional kernel to a MultiKernelModelStak instance based
 *  on the the residue centroid position. */
class KernelCreator {
    static constexpr double fraction = 0.25;
  public:
    KernelCreator() {}
    void operator()( MultiKernelModelStack&, const MultiKernelModelStack&, const Spot& new_kernel ) const;
    void operator()( MultiKernelModel&, const MultiKernelModel&, const Spot& new_kernel ) const;
};

}
}

#endif
