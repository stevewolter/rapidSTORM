#ifndef LIBFITPP_EXPONENTIAL3D_PARAMETERSET_HH
#define LIBFITPP_EXPONENTIAL3D_PARAMETERSET_HH

#include "Exponential3D.hh"
#include <fit++/Exponential_Common.h>
#include <fit++/Exponential_Common_ParameterHelper.hh>

namespace fitpp {
namespace Exponential3D {

template < class Specialization >
struct DerivativeHelper;

template <class Specialization>
struct ParameterHelper;

template <int Ks, int W, int H>
struct Specialization
{
    typedef Model<Ks> Space;
    typedef Exponential::ParameterHelper<Space,W,H,false> BaseParameters;
    typedef ParameterHelper<Specialization> Parameters;
    static const int Width = W, Height = H;
    typedef Eigen::Matrix<double,H,W> Data;
};

template <class Specialization>
struct ParameterHelper 
: public Specialization::BaseParameters
{
    typedef typename Specialization::BaseParameters Base;
    const static int Kernels = Specialization::Space::Kernels;
    Eigen::Matrix<double,Kernels,1> zdx, zdy, dzx, dzy;

    inline bool prepare(
        const typename Specialization::Space::Variables& v,
        const typename Specialization::Space::Constants& c,
        const int x_low, const int y_low
    );
};

template <int Ks, int W, int H>
struct Deriver 
: public DerivativeHelper<Specialization<Ks,W,H> >
{
    typedef Specialization<Ks,W,H> MySpecialization;
    inline bool prepare( 
        const typename MySpecialization::Space::Variables& v,
        const typename MySpecialization::Space::Constants& c,
        const int min_x, const int min_y
    ) {
        bool ok = 
            MySpecialization::Parameters
                ::prepare( v, c, min_x, min_y );
        if (!ok) return false;
        return this->DerivativeHelper<MySpecialization>
               ::prepare();
    }
};

}
}

#endif
