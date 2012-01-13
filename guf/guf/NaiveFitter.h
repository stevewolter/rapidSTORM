#ifndef DSTORM_FITTER_GUF_NAIVEFITTER_INTERFACE_H
#define DSTORM_FITTER_GUF_NAIVEFITTER_INTERFACE_H

#include "Config_decl.h"
#include <dStorm/engine/JobInfo_decl.h>
#include "Data_fwd.h"
#include "FitAnalysis.h"
#include <memory>

namespace dStorm {
namespace guf {

/** Interface for fitting a single function to a data image. */
struct NaiveFitter {
    typedef std::auto_ptr<NaiveFitter> Ptr;
    /** Virtual constructor. 
     *  \tparam KernelCount The number of PSF kernels to use in the function. */
    template <int KernelCount>
    static Ptr
        create( const Config&, const dStorm::engine::JobInfo& );
    virtual ~NaiveFitter() {}
    /** Get a reference to the function's current state. Any call to fit() will
     *  use the current state as the starting point for iteration. */
    virtual FitPosition& fit_position() = 0;
    /** Optimize the current state set by fit_position() using 
     *  Levenberg-Marquardt minimization. 
     *  \returns The new function value, which is the sum of squared residues
     *           for mle == false and the negative likelihood for mle == true.
     **/
    virtual double fit( DataCube& data, bool mle ) = 0;
};

}
}

#endif
