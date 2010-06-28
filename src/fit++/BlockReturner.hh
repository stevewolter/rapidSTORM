#ifndef FITPP_EXPONENTIAL2D_BLOCKRETURNER_HH
#define FITPP_EXPONENTIAL2D_BLOCKRETURNER_HH

#include <Eigen/Core>
namespace fitpp {

template <class Model, int H, int name>
struct BlockReturner {
    typedef Eigen::Matrix<double,H,Model::VarC> VarColumn;
    typedef Eigen::Transpose<typename Eigen::BlockReturnType
        <VarColumn,H,Model::Kernels>::Type> ReturnType;

    ReturnType
    static inline block(VarColumn &matrix, int) {
        return matrix.template block<H,Model::Kernels>(
          0,
          Model::template Parameter<name>::template InKernel<0>::N)
          .transpose();
    }
};
template <class Model, int name>
struct BlockReturner<Model,Eigen::Dynamic,name> {
    typedef Eigen::Matrix<double,Eigen::Dynamic,Model::VarC>
        VarColumn;

    typedef typename Eigen::Transpose<typename Eigen::BlockReturnType<VarColumn>::Type> ReturnType;
    ReturnType
    static inline block(VarColumn &matrix, int dyn_height) {
        return matrix.template block(
          0,
          Model::template Parameter<name>::template InKernel<0>::N,
          dyn_height,
          Model::Kernels)
          .transpose();
    }
};

}

#endif
