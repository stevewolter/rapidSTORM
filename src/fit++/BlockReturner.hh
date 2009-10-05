#ifndef FITPP_EXPONENTIAL2D_BLOCKRETURNER_HH
#define FITPP_EXPONENTIAL2D_BLOCKRETURNER_HH

#include <fit++/Exponential2D.hh>

namespace fitpp {
namespace Exponential2D {

template <int Ks, int PM, int H, Names name>
struct BlockReturner {
    typedef Eigen::Matrix<double,H,For<Ks,PM>::VarC> VarColumn;
    typename Eigen::BlockReturnType<VarColumn,H,Ks>::Type
    static inline block(VarColumn &matrix, int) {
        return matrix.template block<H,Ks>(
          0,
          For<Ks,PM>::template ParamTraits<name>::template Index<0>::N);
    }
};
template <int Ks, int PM, Names name>
struct BlockReturner<Ks,PM,Eigen::Dynamic,name> {
    typedef Eigen::Matrix<double,Eigen::Dynamic,For<Ks,PM>::VarC>
        VarColumn;

    typename Eigen::BlockReturnType<VarColumn>::Type
    static inline block(VarColumn &matrix, int dyn_height) {
        return matrix.template block(
          0,
          For<Ks,PM>::template ParamTraits<name>::template Index<0>::N,
          dyn_height,
          Ks);
    }
};

}
}

#endif
