#ifndef EXPONENTIAL_COMMON_HH
#define EXPONENTIAL_COMMON_HH

#include "Exponential_Common.h"

namespace fitpp {
namespace Exponential {

template <typename Type>
inline Type sq(const Type& a) { return a*a; }

template <int Lines, int Size>
struct PrecalculatedLines<Lines,Size,false>
{
    Eigen::Matrix<double,Lines,Size> val, sqr;
    int width, height;
    
    inline void resize( int size ) {
        if ( Size == Eigen::Dynamic ) {
            val.resize( Lines, size );
            sqr.resize( Lines, size );
        }
    }

    inline void prepare(
        const int low,
        const double scale,
        const Eigen::Matrix<double,Lines,1>& means,
        const Eigen::Matrix<double,Lines,1>& var_Invs
    ) {
        for (int c = 0; c < val.cols(); c++)
            val.col(c) =
                ((-means).cwise()+(c*scale+low)).cwise() * var_Invs;

        sqr = val.cwise().square();
    }
};

template <int Lines, int Size>
struct PrecalculatedLines<Lines,Size,true>
: public PrecalculatedLines<Lines,Size,false> 
{
    typedef PrecalculatedLines<Lines,Size,false> Base;
    Eigen::Matrix<double,Lines,Size> expTerm;

    inline void resize( int size ) {
        Base::resize(size);
        if ( Size == Eigen::Dynamic )
            expTerm.resize( Lines, size );
    }

    inline void prepare(
        const double low,
        const double scale,
        const Eigen::Matrix<double,Lines,1>& means,
        const Eigen::Matrix<double,Lines,1>& var_Invs
    ) {
        Base::prepare(low, scale, means, var_Invs);
        expTerm = (Base::sqr * -0.5).cwise().exp();
    }
};

template <typename Space, int W, int H, bool Corr>
struct ParameterHelper
{
    static const int Ks = Space::Kernels;
    int width, height;
    double shift;
    int x_low;

    PrecalculatedLines<Ks,W,!Corr> xl;
    PrecalculatedLines<Ks,H,!Corr> yl;

    Eigen::Matrix<double,Ks,1> 
        x0, y0, amp, sx, sy, 
        norms, prefactor, sxI, syI,
        ellip, ellipI;
    Eigen::Matrix2d rotation;
    Eigen::Vector2d translation;

    template <int Param> 
    inline static void extract_param(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        Eigen::Matrix<double,Ks,1>& param
    ) {
        typedef typename Space::template Parameter<Param> Traits;
        static const int Index = Traits::template InKernel<0>::N;
        if ( Traits::Variable )
            param = v.template block<Ks,1>(Index, 0);
        else
            param = c.template block<Ks,1>(Index, 0);
    }

    inline void extract( 
        const typename Space::Variables& v,
        const typename Space::Constants& c
    ) {
        rotation = Eigen::Matrix2d::Identity();
        translation = Eigen::Vector2d::Zero();
        shift = Space::ParameterMap::template value<Shift,0>(v, c);
        extract_param<MeanX>( v, c, x0 );
        extract_param<MeanY>( v, c, y0 );
        extract_param<Amplitude>( v, c, amp );
    }
    inline bool check(const int x_low, const int y_low );
    inline void precompute(const int x_low, const int y_low);

    inline void resize( int width, int height ) {
        this->width = width; 
        this->height = height; 
        xl.resize( width );
        yl.resize( height );
    }

    inline bool prepare(
        const typename Space::Variables& v,
        const typename Space::Constants& c,
        const int x_low, const int y_low
    ) {
        extract(v,c);
        if ( ! check( x_low, y_low ) ) {
            DEBUG("Aborted preparation due to failed check");
            return false;
        }
        precompute( x_low, y_low );
        DEBUG("Successfully prepared");
        return true;
    }
};

}
}

#endif
