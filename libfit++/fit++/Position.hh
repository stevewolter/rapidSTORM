#ifndef LIBFITPP_POSITION_H
#define LIBFITPP_POSITION_H

#include <fit++/ResidueMatrix.hh>

namespace fitpp {

template <int VarCount>
class Position 
{
  public:
    typedef Eigen::Matrix<double,VarCount,1> Vector;
    typedef Eigen::Matrix<double,VarCount,VarCount> Matrix;

    Vector parameters;
    Matrix covariances;
    /** The sum of squared residues. */
    double chi_sq;
};

}

#endif
