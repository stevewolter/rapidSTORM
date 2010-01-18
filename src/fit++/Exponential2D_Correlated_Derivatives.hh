#ifndef LIBFITPP_CORRELATED_DERIVATVIES
#define LIBFITPP_CORRELATED_DERIVATVIES

#include <fit++/BlockReturner.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_ParameterHelper.hh>

namespace fitpp {
namespace Exponential2D {

template <int Ks, int PM, int W, int H>
struct DerivativeHelper<Ks,PM,W,H,true>
: public ParameterHelper<Ks,PM,W,H,true>
{
    typedef For<Ks,PM> Space;
    typedef Eigen::Matrix<double,H,W> Data;
    typedef Eigen::Matrix<double,Ks,H> Column;
    typedef Eigen::Matrix<double,H,Space::VarC> VarColumn;

    Column crossprod, covar, expArg,
            expT, kappa, xarray,
            xarraySq;
    VarColumn derivs;

    template <Names name>
    struct Traits {
        typedef typename Space::template ParamTraits<name> Base;
        static const int Variable = Base::Variable;
        template <int Kernel> class Index {
            static const int N = Base::template Index<Kernel>::N;
        };
    };

    inline void resize(const int w, const int h) { 
        this->ParameterHelper<Ks,PM,W,H,true>::resize(w, h); 
        crossprod.resize( Ks, h );
        covar.resize( Ks, h );
        expArg.resize( Ks, h );
        expT.resize( Ks, h );
        kappa.resize( Ks, h );
        xarray.resize( Ks, h );
        xarraySq.resize( Ks, h );
        derivs.resize( h, Space::VarC );
    }
    inline bool prepare() { return true; }

    inline void compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    );
};

template <int Ks, int PM, int W, int H>
inline void
DerivativeHelper<Ks,PM,W,H,true>::compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    )
{
    const Eigen::Matrix<double,Ks,1>& sxy = this->rho;
    const int Width = 
        ((W==Eigen::Dynamic)? this->width : W);
    const int Height = 
        ((H==Eigen::Dynamic)? this->height : H);

    kappa.col(0) = sxy.cwise() * (this->ellipI.cwise() + 1);
    for (int x = 1; x < Height; x++)
        kappa.col(x) = kappa.col(0);

    gradient.setZero();
    hessian.setZero();

    for (int xo = 0; xo < Width; xo++) {
        const Eigen::Matrix<double,Ks,1> x = this->xl.val.col(xo);
        for (int c = 0; c < Height; c++)
            xarray.col(c) = x;
        for (int c = 0; c < Height; c++)
            xarraySq.col(c) = this->xl.sqr.col(xo);

        crossprod =  x.asDiagonal() * this->yl.val;
        covar = 2 * sxy.asDiagonal() * crossprod;
        expArg = - 0.5 * this->ellipI.asDiagonal() *
                   ((this->yl.sqr + covar) + xarraySq);
        expT = expArg.cwise().exp();

        residues.col(xo) = 
            expT.transpose() * this->prefactor -
            (data.col(xo).cwise() - this->shift);
        
        /* Fill derivs vector with derivatives */
        if ( Traits<Shift>::Variable )
            BlockReturner<Ks,PM,H,Shift>::block( derivs, Height )
            .setOnes();
        if ( Traits<MeanX>::Variable )
            BlockReturner<Ks,PM,H,MeanX>::block( derivs, Height )
                .transpose()
                /* E-Term * (x-sxy*y) */
                = expT.cwise() * 
                    ( ( - sxy.asDiagonal() * this->yl.val ) + xarray );
        if ( Traits<MeanY>::Variable )
            BlockReturner<Ks,PM,H,MeanY>::block( derivs, Height )
                .transpose()
                /* E-Term * (y-sxy*x) */
                = expT.cwise() *
                    ( this->yl.val - (sxy.asDiagonal() * xarray) );
        if ( Traits<Amp>::Variable )
            BlockReturner<Ks,PM,H,Amp>::block( derivs, Height )
                .transpose()
                /* E-Term */
                = expT;
        if ( Traits<SigmaX>::Variable )
            BlockReturner<Ks,PM,H,SigmaX>::block( derivs, Height )
                .transpose()
                /* E-Term * ( (x^2 - covar)*ellipI -1 ) */
                /* Equiv: ( -x*ellipI*( 2*y*sxy - x) ) */
                = expT.cwise() *
                    ( (-xarray).cwise() * (this->ellipI.asDiagonal() *
                        ( 2*sxy.asDiagonal()*this->yl.val - xarray )));
        if ( Traits<SigmaY>::Variable )
            BlockReturner<Ks,PM,H,SigmaY>::block( derivs, Height )
                .transpose()
                /* E-Term * ( (y^2 - covar)*ellipI -1 ) */
                = expT.cwise() *
                    ( ( this->ellipI.asDiagonal() *
                        (this->yl.sqr - covar) ).cwise() - 1 );
        if ( Traits<SigmaXY>::Variable )
            BlockReturner<Ks,PM,H,SigmaXY>::block( derivs, Height )
                .transpose()
                /* E-Term * 
                    * - ( expArg + kappa + crossprod ) */
                = -1 * (expT.cwise() *
                    ( expArg + kappa + crossprod ));

        /* Add contributions to gradient and hessian. */
        gradient -= (residues.col(xo).transpose() * derivs).transpose();
        hessian +=  derivs.transpose() * derivs;

    }
    Eigen::Matrix<double,1,Space::VarC> prefactors;
    if ( Traits<Shift>::Variable )
        BlockReturner<Ks,PM,1,Shift>::block( prefactors, 1 ).setOnes();
    if ( Traits<MeanX>::Variable )
        BlockReturner<Ks,PM,1,MeanX>::block( prefactors, 1 )
            = this->prefactor.cwise() * (this->ellipI.cwise() * this->sxI);
    if ( Traits<MeanY>::Variable )
        BlockReturner<Ks,PM,1,MeanY>::block( prefactors, 1 )
            = this->prefactor.cwise() * (this->ellipI.cwise() * this->syI);
    if ( Traits<Amp>::Variable )
        BlockReturner<Ks,PM,1,Amp>::block( prefactors, 1 ) 
            = this->norms;
    if ( Traits<SigmaX>::Variable )
        BlockReturner<Ks,PM,1,SigmaX>::block( prefactors, 1 ) 
            = this->prefactor.cwise() * this->syI;
    if ( Traits<SigmaY>::Variable )
        BlockReturner<Ks,PM,1,SigmaY>::block( prefactors, 1 ) 
            = this->prefactor.cwise() * this->syI;
    if ( Traits<SigmaXY>::Variable )
        BlockReturner<Ks,PM,1,SigmaXY>::block( prefactors, 1 ) 
            = this->prefactor.cwise() * this->ellipI;
            
    gradient.cwise() *= prefactors.transpose();
    hessian.cwise() *= (prefactors.transpose() * prefactors).lazy();
}

}
}

#endif
