#ifndef EXPONENTIAL_UNCORRELATED_DERIVATIVES_HH
#define EXPONENTIAL_UNCORRELATED_DERIVATIVES_HH

#include "BlockReturner.hh"
#include "Exponential_Common.h"

namespace fitpp {
namespace Exponential {

template <typename Super>
struct DerivativeHelper<Super,false>
: public Super::Parameters
{
    static const int H = Super::Height, W = Super::Width;
    typedef Eigen::Matrix<double,H,W> Data;
    typedef typename Super::Parameters Base;
    typedef typename Super::Space Space;

    Eigen::Matrix<double,W,Space::VarC> xparts;
    Eigen::Matrix<double,H,Space::VarC> yparts;

    inline void resize( const int width, const int height ) 
    {
        Base::resize(width,height);
        xparts.resize( width, Space::VarC );
        yparts.resize( height, Space::VarC );
    }

    inline bool prepare();
    inline void compute(
        const Data& data,
        Data& residues,
        typename Space::Vector& gradient,
        typename Space::Matrix& hessian
    ) const;

    template <int Param>
    typename BlockReturner<Space,W,Param>::ReturnType
    getRow() { 
        const int Width = 
            ((W==Eigen::Dynamic)? this->width : W);
        return BlockReturner<Space,W,Param>::block( xparts, Width );
    }
    template <int Param>
    typename BlockReturner<Space,H,Param>::ReturnType
    getColumn() { 
        const int Height = 
            ((H==Eigen::Dynamic)? this->height : H);
        return BlockReturner<Space,H,Param>::block( yparts, Height );
    }
};

template <class Super>
bool 
DerivativeHelper<Super, false>::prepare() 
{
    /* X is negative to automatically give the
        * right gradient sign. */
    if ( Space::template Parameter<Shift>::Variable ) {
        getRow<Shift>().fill( 1 );
        getColumn<Shift>().fill( 1 );
    }

    /* X and Y are asymmetric because X 
        * carries the pre-factors and the negative sign for the 
        * gradient. */
    if ( Space::template Parameter<MeanX>::Variable ) {
        getRow<MeanX>().transpose()
            = - (this->prefactor.cwise() * this->sxI).asDiagonal()
            * (this->xl.expTerm.cwise() * this->xl.val);
        getColumn<MeanX>().transpose()
            = this->yl.expTerm;
    }

    if ( Space::template Parameter<MeanY>::Variable ) {
        getRow<MeanY>().transpose()
            = this->prefactor.asDiagonal() * this->xl.expTerm;
        getColumn<MeanY>().transpose()
            = - this->syI.asDiagonal() * 
                (this->yl.expTerm.cwise() * this->yl.val);
    }

    if ( Space::template Parameter<Amplitude>::Variable ) {
        getRow<Amplitude>().transpose()
            = this->norms.asDiagonal() * this->xl.expTerm;
        getColumn<Amplitude>().transpose() = this->yl.expTerm;
    }

    return true;
}

template <class Model>
void 
DerivativeHelper<Model,false>::compute(
    const Data& data,
    Data& residues,
    typename Space::Vector& gradient,
    typename Space::Matrix& hessian
) const 
{
    /* xlines(c,i) * ylines(i,r) gives the value for
        * the i-th kernel at (r,c) position. This
        * expression sums the contributions and 
        * substracts the shifted data. */
    residues = 
        (data.cwise() - this->shift).lazy() -
            (this->yl.expTerm.transpose() * this->prefactor.asDiagonal() *
                this->xl.expTerm).lazy();

    gradient = (
        (yparts.transpose() * residues).lazy() * xparts).lazy().diagonal();

    /* We compute sum(x,sum(y, dPhi/da_i dPhi/da_j)), which is equal to 
     * ( sum(x, dPhi_x/da_i dPhi_x/da_j) * sum(y, dPhi_y/da_i dPhi_y/da_j) )
     * because the derivative of Phi_1 with respect to parameters from Phi_2
     * is zero and thus dPhi/da_i = dPhi_1/da_i if da_i from Phi_1. */
    hessian = 
        (xparts.transpose() * xparts).lazy().cwise()
        * (yparts.transpose() * yparts).lazy();
}

}
}

#endif
