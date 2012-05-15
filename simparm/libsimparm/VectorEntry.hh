#ifndef SIMPARM_VECTOR_ENTRY_HH
#define SIMPARM_VECTOR_ENTRY_HH

#include "Entry.hh"
#include <simparm/Eigen.hh>

namespace simparm {

template <class Scalar, int Dimensions>
struct vector_entry {
    typedef simparm::Entry< Eigen::Matrix<Scalar, Dimensions, 1, Eigen::DontAlign > > type;
};

template <class Scalar, int Rows, int Cols>
struct matrix_entry {
    typedef simparm::Entry< Eigen::Matrix<Scalar, Rows, Cols, Eigen::DontAlign > > type;
};

}

#endif
