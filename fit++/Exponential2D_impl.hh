#ifndef LIBFITPP_EXPONENTIAL2D_PARAMETERSET_HH
#define LIBFITPP_EXPONENTIAL2D_PARAMETERSET_HH

#include <fit++/Exponential2D.hh>
#include "Exponential_Common.h"
#include "Exponential_Common_ParameterHelper.hh"

namespace fitpp {
namespace Exponential2D {

template <
    class Specialization,
    bool HonorCorrelation>
struct DerivativeHelper;

template <
    int Kernels, int ParameterMask,
    int Width, int Height,
    bool HonorCorrelation>
struct ParameterHelper;

template <int Ks, int PM, bool Corr, int K>
struct FunctionParams;

template <int Ks, int PM, int W, int H, bool UseCorrelation>
struct Specialization
{
    typedef Model<Ks,PM> Space;
    typedef ParameterHelper<Ks,PM,W,H,UseCorrelation> Parameters;
    static const int Width = W, Height = H;
    typedef Eigen::Matrix<double,H,W> Data;
};

template <int Ks, int PM, int W, int H, bool Corr>
struct ParameterHelper 
: public Exponential::ParameterHelper<Model<Ks,PM>,W,H,Corr>
{
    typedef Exponential::ParameterHelper<Model<Ks,PM>,W,H,Corr> Base;
    typedef Model<Ks,PM> Space;
    Eigen::Matrix<double,Ks,1> rho;

    inline bool prepare(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        const int x_low, const int y_low, const int z_layer
    );
};

template <class Specialization>
struct DerivativeHelper<Specialization,true>;

template <class Specialization>
struct DerivativeHelper<Specialization,false>;

template <int Ks, int PM, int W, int H, bool Corr>
struct Deriver 
: public DerivativeHelper<Specialization<Ks,PM,W,H,Corr>, Corr>
{
    typedef Specialization<Ks,PM,W,H,Corr> MySpecialization;
    inline bool prepare( 
        const typename MySpecialization::Space::Variables& v,
        const typename MySpecialization::Space::Constants& c,
        const int min_x, const int min_y, const int z_layer
    ) {
        bool ok = 
            MySpecialization::Parameters
                ::prepare( v, c, min_x, min_y, z_layer );
        if (!ok) return false;
        return this->DerivativeHelper<MySpecialization,Corr>
               ::prepare();
    }
};

}
}

#endif
