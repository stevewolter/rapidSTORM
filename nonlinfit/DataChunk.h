#ifndef NONLINFIT_DATACHUNK_H
#define NONLINFIT_DATACHUNK_H

#include <Eigen/Core>
#include <Eigen/StdVector>

namespace nonlinfit {

template <typename Number, int ChunkSize>
struct DataChunk {
    typedef Eigen::Array<Number, ChunkSize, 1> Output;
    /** The measurements for the matching row in #inputs. */
    Output output;
    /** The value of \f$x \ln x\f$ for each \f$ x \f$ in #output. */
    Output logoutput;
    /** The difference between the measurements and a function's last
        *  evaluation on these data. */
    mutable Output residues;
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
