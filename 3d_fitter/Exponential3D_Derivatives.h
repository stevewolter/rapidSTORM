#ifndef EXPONENTIAL3D_UNCORRELATED_DERIVATIVES_HH
#define EXPONENTIAL3D_UNCORRELATED_DERIVATIVES_HH

#include "debug.h"
#include <fit++/BlockReturner.hh>
#include "Exponential3D_impl.hh"
#include "Exponential3D_ParameterHelper.hh"
#include <fit++/Exponential_Uncorrelated_Derivatives.h>

namespace fitpp {
namespace Exponential3D {

template <class Special>
struct DerivativeHelper
: public Exponential::DerivativeHelper<Special,false>
{
    typedef Exponential::DerivativeHelper<Special,false> Super;
    typedef typename Special::Space Space;
    typedef Exponential::DerivativeHelper<Special,false> Base;
    typedef typename Special::Data Data;

    static const int H = Super::H, W = Super::W,
                     Kernels = Space::Kernels,
                     VarC = Space::VarC;
    int width, height;

    inline void resize( const int width, const int height ) 
    {
        Base::resize(width,height);
        this->width = width;
        this->height = height;
    }

    inline bool prepare();
    inline void compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    ) const;

    template <int Kernel>
    struct ZParam {
        static const int Index = Space::template Parameter<MeanZ>::
            template InKernel<Kernel>::N;
    };
};

template <class Special>
bool 
DerivativeHelper<Special>::prepare() 
{
    this->template getRow<MeanZ>().fill(0);
    this->template getColumn<MeanZ>().fill(1);

    return Base::prepare();
}

template <class Model>
void 
DerivativeHelper<Model>::compute(
    const Data& data,
    Data& residues,
    typename Space::Vector& gradient,
    typename Space::Matrix& hessian
) const 
{
    Base::compute(data,residues,gradient,hessian);

    /* We compute the matrix entries by summing the contributions for
     * each row of the residual matrix. */
    gradient.template block<Kernels,1>(ZParam<0>::Index, 0).fill(0);
    hessian.template block<Kernels,VarC>(ZParam<0>::Index, 0).fill(0);
    static const int Ks = Kernels;
    Eigen::Matrix<double,Ks,H> z_factors(Kernels, (H==Eigen::Dynamic) ? this->height : H);
    Eigen::Matrix<double,Ks,W> x_contribs
            = 2 * (this->zdx.cwise() * ((Model::Use_Holtzer_PSF ? this->sxI.cwise().square() : this->sxI).asDiagonal()
              * ( this->xl.sqr.cwise() - 1 );
    Eigen::Matrix<double,Ks,H> y_contribs
            = 2 * (this->zdy.cwise() * ((Model::Use_Holtzer_PSF ? this->syI.cwise().square() : this->syI)).asDiagonal()
              * ( this->yl.sqr.cwise() - 1 );

    for (int col = 0; col < residues.cols(); ++col ) {
        for (int i = 0; i < z_factors.cols(); ++i) 
            z_factors.col(i) = x_contribs.col(col) + y_contribs.col(i);
            
        Eigen::Matrix<double,Ks,H> derivs
            = (this->xl.expTerm.col(col).cwise() * this->prefactor).asDiagonal()
                  * (this->yl.expTerm.cwise() * z_factors);
        DEBUG("Derivatives for column " << col << " are " << derivs);
        gradient.template block<Kernels,1>(ZParam<0>::Index, 0)
            += (derivs * residues.col(col));
        hessian.template block<Kernels,VarC>(ZParam<0>::Index, 0) 
            += (derivs * this->yparts) * this->xparts.row(col).asDiagonal();
        hessian.template block<Kernels,Kernels>(ZParam<0>::Index, VarC-Kernels) 
            += derivs * derivs.transpose();
    }
    hessian.template block<VarC,Kernels>(0, ZParam<0>::Index)
        = hessian.template block<Kernels,VarC>(ZParam<0>::Index, 0).transpose();
}

}
}

#endif
