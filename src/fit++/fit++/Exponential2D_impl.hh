#ifndef LIBFITPP_EXPONENTIAL2D_PARAMETERSET_HH
#define LIBFITPP_EXPONENTIAL2D_PARAMETERSET_HH

#include <fit++/Exponential2D.hh>

namespace fitpp {
namespace Exponential2D {

template <typename Type>
inline Type sq(const Type& a) throw() { return a*a; }

template <int Lines, int Size, bool ComputeExp>
struct PrecalculatedLines;

template <
    int Kernels, int ParameterMask,
    int Width, int Height,
    bool HonorCorrelation>
struct DerivativeHelper {};

template <
    int Kernels, int ParameterMask,
    int Width, int Height,
    bool HonorCorrelation>
struct ParameterHelper;

template <int Ks, int PM, bool Corr, int K>
struct FunctionParams;

template <int Lines, int Size>
struct PrecalculatedLines<Lines,Size,false>
{
    Eigen::Matrix<double,Lines,Size> val, sqr;
    int width, height;
    
    inline void resize( int size ) throw() {
        if ( Size == Eigen::Dynamic ) {
            val.resize( Lines, size );
            sqr.resize( Lines, size );
        }
    }

    inline void prepare(
        const int low,
        const Eigen::Matrix<double,Lines,1>& means,
        const Eigen::Matrix<double,Lines,1>& var_Invs
    ) throw() {
        for (int c = 0; c < val.cols(); c++)
            val.col(c) =
                ((-means).cwise()+(c+low)).cwise() * var_Invs;

        sqr = val.cwise().square();
    }
};

template <int Lines, int Size>
struct PrecalculatedLines<Lines,Size,true>
: public PrecalculatedLines<Lines,Size,false> 
{
    typedef PrecalculatedLines<Lines,Size,false> Base;
    Eigen::Matrix<double,Lines,Size> expTerm;

    inline void resize( int size ) throw() {
        Base::resize(size);
        if ( Size == Eigen::Dynamic )
            expTerm.resize( Lines, size );
    }

    inline void prepare(
        const int low,
        const Eigen::Matrix<double,Lines,1>& means,
        const Eigen::Matrix<double,Lines,1>& var_Invs
    ) throw() {
        Base::prepare(low, means, var_Invs);
        expTerm = (Base::sqr * -0.5).cwise().exp();
    }
};

template <int Ks, int PM, int W, int H, bool Corr>
struct ParameterHelper {
    typedef For<Ks,PM> Space;

    template <int Param> 
    inline static void extract_param(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        Eigen::Matrix<double,Ks,1>& param
    ) throw() {
        typedef typename Space::template ParamTraits<Param> Traits;
        if ( Traits::Variable )
            param = v.template block<Ks,1>
                    (Traits::template Index<0>::N, 0);
        else
            param = c.template block<Ks,1>
                    (Traits::template Index<0>::N, 0);
    }

    int width, height;
    int shift;
    int x_low;

    PrecalculatedLines<Ks,W,!Corr> xl;
    PrecalculatedLines<Ks,H,!Corr> yl;

    Eigen::Matrix<double,Ks,1> 
        x0, y0, amp, sx, sy, rho,
        norms, prefactor,
        ellip, ellipI, sxI, syI;

    inline void resize( int width, int height ) throw() {
        this->width = width; 
        this->height = height; 
        xl.resize( width );
        yl.resize( height );
    }

    inline bool prepare(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        const int x_low, const int y_low
    ) throw();
};

template <int Ks, int PM, int W, int H>
struct DerivativeHelper<Ks,PM,W,H,true>;

template <int Ks, int PM, int W, int H>
struct DerivativeHelper<Ks,PM,W,H,false>;

template <int Ks, int PM, int W, int H, bool Corr>
struct Deriver 
: public DerivativeHelper<Ks,PM,W,H,Corr>
{
    typedef For<Ks,PM> Space;
    inline bool prepare( 
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        const int min_x, const int min_y
    ) throw() {
        bool ok = 
            this->ParameterHelper<Ks,PM,W,H,Corr>
                ::prepare( v, c, min_x, min_y );
        if (!ok) return false;
        return this->DerivativeHelper<Ks,PM,W,H,Corr>
               ::prepare();
    }
};

}
}

#endif
