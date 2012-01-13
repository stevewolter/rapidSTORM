#include "FitAnalysis.h"

namespace dStorm {
namespace guf {

/** Function object to add an additional kernel to a FitPosition instance based
 *  on the the residue centroid position. */
class KernelCreator {
    static const double fraction = 0.25;
  public:
    KernelCreator() {}
    void operator()( FitPosition&, const FitPosition&, const Spot& new_kernel ) const;
    void operator()( FittedPlane&, const FittedPlane&, const Spot& new_kernel ) const;
};

}
}
