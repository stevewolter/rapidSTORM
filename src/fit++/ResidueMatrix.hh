#ifndef LIBFITPP_RESIDUE_MATRIX_H
#define LIBFITPP_RESIDUE_MATRIX_H

#include <Eigen/Core>

namespace fitpp {

template <int Rows, int Cols>
struct ResidueMatrix {
    typedef Eigen::Matrix<double,Rows,Cols> Matrix;
  private:
    int center_element_row, center_element_col;
    Matrix v;
  public:
    double operator()(int row, int col) const throw()
        { return v(center_element_row + row,
                   center_element_col + col); }

    int min_row() const throw() 
        { return -center_element_row; }
    int max_row() const throw() 
        { return v.rows()-1-center_element_row; }
    int min_col() const throw() 
        { return -center_element_col; }
    int max_col() const throw() 
        { return v.cols()-1-center_element_col; }

    const Matrix& matrix() const throw() 
        { return v; }
    Matrix& matrix() throw() 
        { return v; }

    void set_size( int width, int height ) throw() {
        v.resize( height, width );
    }

    void set_center( int x, int y ) throw() {
        center_element_row = y;
        center_element_col = x;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}

#endif
