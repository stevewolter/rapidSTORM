#ifndef LOCPREC_BIPLANE_ALIGNMENT_DERIVATOR_H
#define LOCPREC_BIPLANE_ALIGNMENT_DERIVATOR_H

#include "impl.h"
#include <gsl/gsl_vector.h>
#include <gsl/gsl_multifit_nlin.h>

namespace locprec {
namespace biplane_alignment {

class Derivator {
    typedef Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor | Eigen::DontAlign> GSLMatrix;

    const MotionModel& model;
    const std::vector< Source::Image >& planes;
    gsl_multifit_fdfsolver* solver;
    gsl_multifit_function_fdf function;
    int pixel_count;
    double scale_factor;
    void (Derivator::* evaluator)(const Eigen::VectorXd& parameters,
        Eigen::VectorXd::UnalignedMapType* function,
        GSLMatrix::UnalignedMapType* jacobian,
        std::vector< Source::Image >::const_iterator begin,
        std::vector< Source::Image >::const_iterator end );

    void image_gradient( const Source::Plane&, int line, Eigen::MatrixXd& );
    void evaluate( 
        const Eigen::VectorXd& parameters,
        Eigen::VectorXd::UnalignedMapType* function,
        GSLMatrix::UnalignedMapType* jacobian );
    template <typename MotionModel>
    void evaluate_range( 
        const Eigen::VectorXd& parameters,
        Eigen::VectorXd::UnalignedMapType* function,
        GSLMatrix::UnalignedMapType* jacobian,
        std::vector< Source::Image >::const_iterator begin,
        std::vector< Source::Image >::const_iterator end );

    static int f( const gsl_vector * x, void * params, gsl_vector * f );
    static int df( const gsl_vector * x, void * params, gsl_matrix * J );
    static int fdf( const gsl_vector * x, void * params, gsl_vector * f, gsl_matrix *J );

    Eigen::VectorXd improve_parameters_on_half_scale(Eigen::VectorXd guess) const;

  public:
    Derivator( const MotionModel& model, const std::vector< Source::Image >& planes, double scale_factor = 1 );
    Eigen::VectorXd fit( const Eigen::VectorXd& initial_guess );
    ~Derivator();
};

}
}

#endif
