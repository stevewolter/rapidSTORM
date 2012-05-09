#ifndef DSTORM_GUF_PSF_EVALUATOR_H
#define DSTORM_GUF_PSF_EVALUATOR_H

#include "fwd.h"
#include "BaseEvaluator.h"
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/Xs.h>
#include "parameters.h"
#include <nonlinfit/Evaluator.h>

namespace dStorm {
namespace measured_psf {

template <typename Num, typename Expression, int ChunkSize>
struct JointEvaluator
: public Parameters<Num, Expression >
{
    Eigen::Array<Num, ChunkSize, 2> normed, squared;
    Eigen::Array<Num, ChunkSize, 1> expT;

    JointEvaluator() {}
    JointEvaluator( const Expression& e ) : Parameters<Num, Expression >(e) {}
    void prepare_chunk( const Eigen::Array<Num,ChunkSize,2>& xs ) 
    {
        normed = (xs.rowwise() - this->spatial_mean.transpose())
                .matrix() * this->sigmaI.matrix().asDiagonal();
        squared = normed.square();
        expT = (squared.rowwise().sum() * -0.5).exp() * this->prefactor;
    }

    void value( Eigen::Array<Num,ChunkSize,1>& result ) 
        { result = this->expT; }
    void add_value( Eigen::Array<Num,ChunkSize,1>& result ) 
        { result += this->expT; }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim,LengthUnit> ) {
        target = (expT * normed.col(Dim)) * -this->sigmaI[Dim];
    }
    template <typename Target, int Dim>
    void derivative( Target target, Mean<Dim> ) {
        target = (expT * normed.col(Dim)) * this->sigmaI[Dim];
    }
    template <typename Target>
    void derivative( Target target, const MeanZ& ) {
        target = ((squared - 1).matrix() * -this->z_deriv_prefactor.matrix())
                    .array() * expT;
    }
    template <typename Target, int Dim>
    void derivative( Target target, const ZPosition<Dim>& ) {
        target = ((squared.col(Dim) - 1) * this->z_deriv_prefactor[Dim]) * expT;
    }

    template <typename Target>
    void derivative( Target target, const Amplitude& ) {
        target = expT / this->amplitude;
    }

    template <typename Target, int Dim>
    void derivative( Target target, const BestSigma<Dim>& ) {
        target = (squared.col(Dim) - 1) * expT * this->sigma_deriv[Dim];
    }

    template <typename Target>
    void derivative( Target target, const Prefactor& ) {
        target = expT / this->transmission;
    }

    template <typename Target, int Dim, int Term>
    void derivative( Target target, const DeltaSigma<Dim,Term>& ) { 
        target = this->delta_z_deriv_prefactor(Dim,Term) * 
            ((squared.col(Dim) - 1) * expT);
    }
};

}
}

namespace nonlinfit {

#define DSTORM_GUF_PSF_JOINT_SPECIALIZATION(Expression) \
template <typename Num, int ChunkSize> \
struct get_evaluator< \
    Expression, \
    plane::Joint<Num,ChunkSize,dStorm::gaussian_psf::XPosition,dStorm::gaussian_psf::YPosition> > \
{ \
    typedef typename boost::mpl::if_c< ChunkSize == 1, \
        dStorm::gaussian_psf::ReferenceEvaluator< Expression, Num, dStorm::gaussian_psf::XPosition, dStorm::gaussian_psf::YPosition >, \
        dStorm::gaussian_psf::JointEvaluator< Num, Expression, ChunkSize > \
    >::type type;  \
};
DSTORM_GUF_PSF_JOINT_SPECIALIZATION(dStorm::gaussian_psf::Polynomial3D)
DSTORM_GUF_PSF_JOINT_SPECIALIZATION(dStorm::gaussian_psf::No3D)
DSTORM_GUF_PSF_JOINT_SPECIALIZATION(dStorm::gaussian_psf::Spline3D)
#undef DSTORM_GUF_PSF_JOINT_SPECIALIZATION

}

#endif
