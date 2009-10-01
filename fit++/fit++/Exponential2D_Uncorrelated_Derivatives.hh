#ifndef EXPONENTIAL2D_UNCORRELATED_DERIVATIVES_HH
#define EXPONENTIAL2D_UNCORRELATED_DERIVATIVES_HH

#include <fit++/BlockReturner.hh>
#include <fit++/Exponential2D_impl.hh>
#include <fit++/Exponential2D_ParameterHelper.hh>

namespace fitpp {
namespace Exponential2D {

template <int Ks, int PM, int W, int H>
struct DerivativeHelper<Ks,PM,W,H,false>
: public ParameterHelper<Ks,PM,W,H,false>
{
    typedef For<Ks,PM> Space;
    typedef Eigen::Matrix<double,H,W> Data;
    typedef ParameterHelper<Ks,PM,W,H,false> Base;

    Eigen::Matrix<double,W,Space::VarC> xparts;
    Eigen::Matrix<double,H,Space::VarC> yparts;

    inline void resize( const int width, const int height ) 
        throw()
    {
        Base::resize(width,height);
        xparts.resize( width, Space::VarC );
        yparts.resize( height, Space::VarC );
    }

    inline bool prepare() throw(); 
    inline void compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    ) const throw();

};

template <int Ks, int PM, int W, int H>
bool 
DerivativeHelper<Ks,PM,W,H,false>::prepare() throw()
{
    const int Width = 
        ((W==Eigen::Dynamic)? this->width : W);
    const int Height = 
        ((H==Eigen::Dynamic)? this->height : H);

    /* X is negative to automatically give the
        * right gradient sign. */
    if ( Space::template ParamTraits<Shift>::Variable ) {
        BlockReturner<Ks,PM,W,Shift>::block( xparts, Width ).fill( -1 );
        BlockReturner<Ks,PM,H,Shift>::block( yparts, Height ).fill( 1 );
    }

    /* X and Y are asymmetric because X 
        * carries the pre-factors and the negative sign for the 
        * gradient. */
    if ( Space::template ParamTraits<MeanX>::Variable ) {
        BlockReturner<Ks,PM,W,MeanX>::block( xparts, Width ).transpose()
            = -  (this->prefactor.cwise() * this->sxI).asDiagonal()
            * (this->xl.expTerm.cwise() * this->xl.val);
        BlockReturner<Ks,PM,H,MeanX>::block( yparts, Height).transpose()
            = this->yl.expTerm;
    }

    if ( Space::template ParamTraits<MeanY>::Variable ) {
        BlockReturner<Ks,PM,W,MeanY>::block( xparts, Width ).transpose()
            = - this->prefactor.asDiagonal() * this->xl.expTerm;
        BlockReturner<Ks,PM,H,MeanY>::block( yparts, Height).transpose()
            = this->syI.asDiagonal() * 
                (this->yl.expTerm.cwise() * this->yl.val);
    }

    if ( Space::template ParamTraits<Amplitude>::Variable ) {
        BlockReturner<Ks,PM,W,Amp>::block( xparts, Width ).transpose()
            = -  this->norms.asDiagonal() * this->xl.expTerm;
        BlockReturner<Ks,PM,H,Amp>::block( yparts, Height).transpose()
            = this->yl.expTerm;
    }

    return true;
}

template <int Ks, int PM, int W, int H>
void 
DerivativeHelper<Ks,PM,W,H,false>::compute(
    const Data& data,
    Data& residues,
    typename Space::Vector& gradient,
    typename Space::Matrix& hessian
) const throw() 
{
    /* xlines(c,i) * ylines(i,r) gives the value for
        * the i-th kernel at (r,c) position. This
        * expression sums the contributions and 
        * substracts the shifted data. */
    residues = 
        (this->yl.expTerm.transpose() * this->prefactor.asDiagonal() *
        this->xl.expTerm).lazy()
        - (data.cwise() - this->shift).lazy();

    gradient = (
        (yparts.transpose() * residues).lazy() * xparts).lazy().diagonal();
    hessian = 
        (xparts.transpose() * xparts).lazy().cwise()
        * (yparts.transpose() * yparts).lazy();
}

}
}

#endif
