#ifndef LIBFITPP_EXPONENTIAL3D_PARAMETERSET_HH
#define LIBFITPP_EXPONENTIAL3D_PARAMETERSET_HH

#include "Exponential3D.hh"
#include <fit++/Exponential_Common.h>
#include <fit++/Exponential_Common_ParameterHelper.hh>

namespace fitpp {
namespace Exponential3D {

template < class Specialization >
struct DerivativeHelper;

template <int Kernels, int Widening, int W, int H>
struct ParameterHelper;

template <class Model, int W, int H>
struct Specialization
{
    typedef Model Space;
    typedef Exponential::ParameterHelper<Space,W,H,false> BaseParameters;
    typedef ParameterHelper<Model::Kernels, Model::Widening,W,H> Parameters;
    static const int Width = W, Height = H;
    typedef Eigen::Matrix<double,H,W> Data;
};

template <int Kernels, int Widening, int W, int H>
struct ParameterHelper 
: public Specialization<Model<Kernels,Widening>,W,H>::BaseParameters
{
    typedef Specialization<Model<Kernels,Widening>,W,H> MySpecialization;
    typedef typename MySpecialization::BaseParameters Base;
    Eigen::Matrix<double,Kernels,2> z_deriv_prefactor;

    inline bool prepare(
        const typename MySpecialization::Space::Variables& v,
        const typename MySpecialization::Space::Constants& c,
        const int x_low, const int y_low, const int z_layer
    );

  protected:
    typedef Model<Kernels,Widening> Space;
    template <int ParamX, int ParamY> 
    inline static void extract_param_xy(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        Eigen::Matrix<double,Kernels,2>& param
    ) 
    {
        typedef typename Space::template Parameter<ParamX> Traits;
        static const int IndexX = Space::template Parameter<ParamX>::template InKernel<0>::N;
        static const int IndexY = Space::template Parameter<ParamY>::template InKernel<0>::N;
        if ( Traits::Variable ) {
            param.col(0) = v.template block<Kernels,1>(IndexX, 0);
            param.col(1) = v.template block<Kernels,1>(IndexY, 0);
        } else {
            param.col(0) = c.template block<Kernels,1>(IndexX, 0);
            param.col(1) = c.template block<Kernels,1>(IndexY, 0);
        }
    }
    
};

template <class Model, int W, int H>
struct Deriver 
: public DerivativeHelper<Specialization<Model,W,H> >
{
    typedef Specialization<Model,W,H> MySpecialization;
    inline bool prepare( 
        const typename MySpecialization::Space::Variables& v,
        const typename MySpecialization::Space::Constants& c,
        const int min_x, const int min_y, const int z_layer
    ) {
        bool ok = 
            MySpecialization::Parameters
                ::prepare( v, c, min_x, min_y, z_layer );
        if (!ok) return false;
        return this->DerivativeHelper<MySpecialization>
               ::prepare();
    }
};

}
}

#endif
