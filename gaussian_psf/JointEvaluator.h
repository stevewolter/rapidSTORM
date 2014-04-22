#ifndef DSTORM_GUF_PSF_EVALUATOR_H
#define DSTORM_GUF_PSF_EVALUATOR_H

#include "gaussian_psf/fwd.h"
#include "gaussian_psf/BaseEvaluator.h"
#include <nonlinfit/plane/fwd.h>
#include <nonlinfit/Xs.h>
#include "gaussian_psf/parameters.h"
#include <nonlinfit/Evaluator.h>

namespace dStorm {
namespace gaussian_psf {

template <typename Number, typename Expression, int ChunkSize>
struct JointEvaluator
: private Parameters<Number, Expression >,
  public nonlinfit::Term<Number, ChunkSize>
{
    JointEvaluator() {}
    JointEvaluator( const nonlinfit::plane::GenericData* generic_data,
                    const std::vector<Eigen::Matrix<Number, ChunkSize, 2>>* data,
                    const Expression& e )
        : Parameters<Number, Expression >(e), generic_data_(generic_data), data_(data) {}

    bool StartIteration() OVERRIDE {
        chunk_ = data_.begin();
        return this->prepare_iteration(*generic_data_);
    }

    void ComputeNextValuesAndDerivatives(
        Eigen::Matrix<Number, ChunkSize, 1>* values,
        Eigen::Matrix<Number, ChunkSize, Eigen::Dynamic>* jacobian,
        int* offset) OVERRIDE {
        assert(chunk_ != data_.end());
        prepare_chunk(*chunk_);
        add_value(*values);
        boost::mpl::for_each<typename Expression::Variables>(
            boost::bind(&JointEvaluator::set_variable, this, jacobian, offset)
        );
        ++chunk_;
    }

  private:
    Eigen::Array<Number, ChunkSize, 2> normed, squared;
    Eigen::Array<Number, ChunkSize, 1> expT;
    const nonlinfit::plane::GenericData* generic_data_;
    const std::vector<Eigen::Matrix<Number, ChunkSize, 2>>* data_;
    typename std::vector<Eigen::Matrix<Number, ChunkSize, 2>>::const_iterator chunk_;

    void prepare_chunk( const Eigen::Array<Number,ChunkSize,2>& xs ) 
    {
        normed = (xs.rowwise() - this->spatial_mean.transpose())
                .matrix() * this->sigmaI.matrix().asDiagonal();
        squared = normed.square();
        expT = (squared.rowwise().sum() * -0.5).exp() * this->prefactor;
    }

    void value( Eigen::Array<Number,ChunkSize,1>& result ) 
        { result = this->expT; }
    void add_value( Eigen::Array<Number,ChunkSize,1>& result ) 
        { result += this->expT; }

    template <typename Variable>
    void set_derivative(
        Eigen::Matrix<Number, ChunkSize, Eigen::Dynamic>* target_matrix,
        int* column,
        Variable variable) {
        derivative(target_matrix->col(*column), variable);
        *column += 1;
    }

    template <typename Target, int Dim>
    void derivative( Target target, nonlinfit::Xs<Dim> ) {
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

#endif
