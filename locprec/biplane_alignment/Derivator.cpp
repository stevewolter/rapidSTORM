#define BOOST_DISABLE_ASSERTS
#include "Derivator.h"
#include "MotionModels.h"
#include <Eigen/Core>
#include <dStorm/image/slice.h>
#include <dStorm/image/constructors.h>
#include <gsl/gsl_blas.h>
#include <boost/thread/thread.hpp>

#define VERBOSE
#ifdef VERBOSE
#undef VERBOSE
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/helpers/DisplayDataSource_impl.h>
#include <dStorm/image/minmax.h>
#include <dStorm/image/convert.h>
#include <boost/lexical_cast.hpp>
#undef DEBUG
#undef SIMPARM_DEBUG
#define VERBOSE
#include <dStorm/debug.h>
#endif

namespace Eigen {
template <>
struct NumTraits<unsigned short> : public NumTraits<int> {};
}

std::ostream& operator<<( std::ostream& o, const gsl_vector& d )
{
    for (unsigned int i = 0; i < d.size; ++i)
        o << ((i==0)?"":" ") << d.data[i*d.stride];
    return o;
}

std::ostream& operator<<( std::ostream& o, const gsl_matrix& d )
{
    for (unsigned int i = 0; i < d.size1; ++i) {
        for (unsigned int j = 0; j < d.size2; ++j)
            o << ((j==0)?"":" ") << d.data[i*d.tda+j];
        o << "\n";
    }
    return o;
}

namespace locprec {
namespace biplane_alignment {

#ifdef VERBOSE
static int output_image = 0;
#endif

int Derivator::f( const gsl_vector * x, void * params, gsl_vector * f )
{
    assert( f->stride == 1 );
    assert( x->stride == 1 );
    Eigen::Map<Eigen::VectorXd> d( f->data, f->size );
    DEBUG("Evaluating function for " << Eigen::VectorXd::Map( x->data, x->size ).transpose() );
    static_cast<Derivator*>(params)->evaluate( Eigen::VectorXd::Map( x->data, x->size ), &d, NULL );
    DEBUG( "Evaluated function to chisq " << d.transpose() * d );
    return GSL_SUCCESS;
}

int Derivator::df( const gsl_vector * x, void * params, gsl_matrix * J )
{
    assert( J->size2 == J->tda );
    assert( x->stride == 1 );
    Eigen::Map<GSLMatrix> eJ( J->data, J->size1, J->size2 );
    DEBUG("Evaluating derivatives for " << Eigen::VectorXd::Map( x->data, x->size ).transpose() );
    static_cast<Derivator*>(params)->evaluate( Eigen::VectorXd::Map( x->data, x->size ), NULL, &eJ );
    DEBUG( "Evaluated derivatives" );
    return GSL_SUCCESS;
}

int Derivator::fdf( const gsl_vector * x, void * params, gsl_vector* f, gsl_matrix * J )
{
    assert( J->size2 == J->tda );
    assert( x->stride == 1 );
    Eigen::Map<Eigen::VectorXd> d( f->data, f->size );
    Eigen::Map<GSLMatrix> eJ( J->data, J->size1, J->size2 );
    DEBUG("Evaluating function and derivatives for " << Eigen::VectorXd::Map( x->data, x->size ).transpose() );
    static_cast<Derivator*>(params)->evaluate( Eigen::VectorXd::Map( x->data, x->size ), &d, &eJ );
    DEBUG( "Evaluated function to chisq " << d.transpose() * d );
    return GSL_SUCCESS;
}

using namespace boost::units;

template <typename Pixel>
Eigen::Map< Eigen::Matrix<Pixel, Eigen::Dynamic, 1> >
eigen_map( const dStorm::Image<Pixel,1>& img )
{
    return Eigen::Map< Eigen::Matrix<Pixel, Eigen::Dynamic, 1> >( const_cast<Pixel*>(img.ptr()), img.width_in_pixels() );
        
}

template <typename Pixel>
Eigen::Map< Eigen::Matrix<Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> >
eigen_map( const dStorm::Image<Pixel,2>& img )
{
    return Eigen::Matrix<Pixel, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>::Map( 
        const_cast<Pixel*>(img.ptr()), img.height_in_pixels(), img.width_in_pixels() );
        
}

void Derivator::image_gradient( const Source::Plane& warped_image, int line, Eigen::MatrixXd& target )
{
    target.fill(0);
    if ( line > 0 && line < warped_image.height_in_pixels() - 1 )
        for (int x = 1; x < warped_image.width_in_pixels()-1; ++x) {
            for (int d = -1; d <= 1; d += 2) {
                /* Sobel gradients */
                target(0,x) += d * (warped_image(x+d,line-1) + 2*warped_image(x+d,line) + warped_image(x+d,line+1)) / 4;
                target(1,x) += d * (warped_image(x-1,line+d) + 2*warped_image(x,line) + warped_image(x+1,line+d)) / 4;
            }
            //DEBUG("Set X gradient for " << x << "," << line << " to " << target(0,x));
        }
    
}

#ifdef VERBOSE
template <typename Pixel>
inline void save_image( const dStorm::Image<Pixel,2>& image, std::string filename )
{
    dStorm::Display::Change c(0);
    c.do_clear = true;
    c.clear_image.background = dStorm::Pixel::Black();
    c.display_normalized( image );
    c.resize_image.keys.clear();
    c.changed_keys.clear();
    dStorm::Display::Manager::getSingleton().store_image( filename, c );
}
#endif

void Derivator::evaluate( const Eigen::VectorXd& parameters, Eigen::Map<Eigen::VectorXd>* function, Eigen::Map<GSLMatrix>* jacobian )
{
    DEBUG("Evaluating function");
    std::vector< Source::Image >::const_iterator next_slice = planes.begin(), end_of_slice;
    boost::ptr_vector< boost::thread > threads;
    int slice_size = planes.size() / 4;
    for (int i = 0; i < 3; ++i) {
        end_of_slice = next_slice + slice_size;
        DEBUG("Starting subthread " << i);
        if ( next_slice != end_of_slice )
            threads.push_back( new boost::thread(
                    evaluator, this, parameters, function,
                    jacobian, next_slice, end_of_slice ) );
        DEBUG("Started subthread " << i);
        next_slice = end_of_slice;
    }
    DEBUG("Running main computation");
    (this->*evaluator)( parameters, function, jacobian, next_slice, planes.end() );
    DEBUG("Ran main computation");
    while ( ! threads.empty() ) {
        threads.back().join();
        threads.pop_back();
    }
    DEBUG("Joined subthreads");
}

template <typename MyMotionModel>
#if ( defined(_WIN32) || defined(_WIN64) ) 
__attribute__ (( force_align_arg_pointer))
#endif
void Derivator::evaluate_range( 
    const Eigen::VectorXd& parameters,
    Values* function, Jacobian* jacobian,
    std::vector< Source::Image >::const_iterator begin,
    std::vector< Source::Image >::const_iterator end )
{
    MotionModel::Motion motion = model.get_motion( parameters );
    MotionModel::FieldJacobian field_jacobian;
    MyMotionModel::initialize_field_jacobian( field_jacobian );

    DEBUG("For iteration " << output_image << " have parameters " << parameters.transpose() << " with matrix\n" << motion);

    for ( std::vector< Source::Image >::const_iterator image = begin; image != end; ++image )
    {
        int data_n = (image - planes.begin()) * image->width_in_pixels() * image->height_in_pixels();
        Source::Plane plane = image->slice(2, 1 * camera::pixel), target = image->slice(2, 0 * camera::pixel);
#ifdef VERBOSE
        Source::Plane difference_image( plane.sizes() );
        dStorm::Image<double,2> x_gradient( plane.sizes() ), y_gradient( plane.sizes() );
        std::vector< dStorm::Image<double,2> > steepest_descent;
        for (int i = 0; i < parameters.rows(); ++i)
            steepest_descent.push_back( dStorm::Image<double,2>( plane.sizes() ) );
#endif
        Eigen::MatrixXd gradient( 2, plane.width_in_pixels() );
        Source::Plane warped_image( plane.sizes() );
        Source::apply_motion( plane, motion, warped_image );
        for ( int line = 0; line < warped_image.height_in_pixels(); ++line ) {
            const int W = warped_image.width_in_pixels();
            if ( function ) {
                function->block( data_n, 0, W, 1 ) 
                    = (eigen_map( warped_image.slice(1, line * camera::pixel) ).cast<double>() -
                       eigen_map( target.slice(1, line * camera::pixel )).cast<double>());
#ifdef VERBOSE
                eigen_map( difference_image.slice(1, line * camera::pixel ) )
                    = function->block( data_n, 0, W, 1 ).cast<float>();
#endif
            }
            if ( jacobian ) {
                image_gradient( warped_image, line, gradient );
#ifdef VERBOSE
                eigen_map( x_gradient.slice(1, line * camera::pixel ) ) = gradient.row(0);
                eigen_map( y_gradient.slice(1, line * camera::pixel ) ) = gradient.row(1);
#endif
                for (int x = 0; x < W; ++x) {
                    MyMotionModel::set_field_jacobian( field_jacobian, x, line );
                    jacobian->block( data_n + x, 0, 1, parameters.rows() )
                        = - gradient.col(x).transpose() * field_jacobian;
#ifdef VERBOSE
                    for (int i = 0; i < field_jacobian.cols(); ++i)
                        steepest_descent[i](x, line) = (*jacobian)( data_n + x, i );
#endif
                }
            }
            data_n += W;
        }
#ifdef VERBOSE
        if ( image == planes.begin() ) {
            std::string ident = "_" + boost::lexical_cast<std::string>(output_image++);
            save_image(plane, "original_image" + ident + ".png");
            save_image(target, "target_image" + ident + ".png");
            save_image(warped_image, "warped_image" + ident + ".png");
            if ( function )
                save_image(difference_image, "difference_image" + ident + ".png");
            if ( jacobian ) {
                DEBUG("X gradient at 1,3 is " << x_gradient(1,3));
                save_image(x_gradient, "x_gradient" + ident + ".png");
                save_image(y_gradient, "y_gradient" + ident + ".png");
                for (int i = 0; i < field_jacobian.cols(); ++i)
                    save_image(steepest_descent[i], "steepest_descent_" + boost::lexical_cast<std::string>(i) + ident + ".png");
            }
        }
#endif
    }
}

Derivator::Derivator( const MotionModel& model, const std::vector< Source::Image >& planes, double scale_factor )
: model(model), planes(planes), solver(NULL), scale_factor(scale_factor)
{
    if ( dynamic_cast<const NoMotion*>(&model) )
        evaluator = &Derivator::evaluate_range<NoMotion>;
    else if ( dynamic_cast<const Translation*>(&model) )
        evaluator = &Derivator::evaluate_range<Translation>;
    else if ( dynamic_cast<const ScaledTranslation*>(&model) )
        evaluator = &Derivator::evaluate_range<ScaledTranslation>;
    else if ( dynamic_cast<const Euclidean*>(&model) )
        evaluator = &Derivator::evaluate_range<Euclidean>;
    else if ( dynamic_cast<const Similarity*>(&model) )
        evaluator = &Derivator::evaluate_range<Similarity>;
    else if ( dynamic_cast<const Affine*>(&model) )
        evaluator = &Derivator::evaluate_range<Affine>;
    else
        throw std::logic_error("Unknown motion model in locprec::biplane_alignment::Derivator::Derivator");

    pixel_count = planes.size() * planes.front().size_in_pixels() / planes.front().depth_in_pixels();
    solver = gsl_multifit_fdfsolver_alloc( gsl_multifit_fdfsolver_lmder, pixel_count, model.parameter_count() );
    function.f = &Derivator::f;
    function.df = &Derivator::df;
    function.fdf = &Derivator::fdf;
    function.params = this;
}

Derivator::~Derivator() 
{
    gsl_multifit_fdfsolver_free( solver );
}

Eigen::VectorXd Derivator::fit( const Eigen::VectorXd& initial_guess )
{
    int rv;

    Eigen::VectorXd guess = initial_guess;
    if ( planes.size() > 0 && (planes.front().width() * planes.front().height()).value() > 128 )
        guess = improve_parameters_on_half_scale(guess);

    function.n = pixel_count;
    function.p = initial_guess.rows();

    gsl_vector v;
    v.size = initial_guess.rows();
    v.stride = 1;
    v.data = const_cast<double*>(guess.data());
    v.block = NULL;
    v.owner = 0;

    rv = gsl_multifit_fdfsolver_set( solver, &function, &v );
    assert( rv == GSL_SUCCESS );
    for (int i = 0; i < 200; ++i) {
        DEBUG( "Position before iteration " << i << ": " << *gsl_multifit_fdfsolver_position(solver) );
        double chi_sq;
        gsl_blas_ddot(solver->f, solver->f, &chi_sq);
        DEBUG( "Chi square is " << chi_sq );
        int rv = gsl_multifit_fdfsolver_iterate( solver );
        if ( rv == GSL_SUCCESS )
            if ( gsl_multifit_test_delta(solver->dx, solver->x, 0.1, 0.001) == GSL_SUCCESS )
                break;
            else
                continue;
        else if ( rv == GSL_ENOPROG )
            break;
        else
            throw std::runtime_error(gsl_strerror(rv));
    }
    DEBUG( "Final position: " << *gsl_multifit_fdfsolver_position(solver) );
    gsl_vector* result = gsl_multifit_fdfsolver_position(solver);
    assert(  result->stride == 1 );
    return Eigen::VectorXd::Map( result->data, result->size );
}

Eigen::VectorXd Derivator::improve_parameters_on_half_scale(Eigen::VectorXd initial_guess) const
{
    const int f = 2;
    std::vector< Source::Image > half_scale;

    for ( std::vector< Source::Image >::const_iterator i = planes.begin(); i != planes.end(); ++i )
    {
        Source::Image::Size sz = i->sizes();
        sz.x() /= f;
        sz.y() /= f;
        Source::Image t( sz );
        t.fill(0);
        for ( Source::Image::iterator j = t.begin(); j != t.end(); ++j )
            for (int dx = 0; dx < 2; ++dx) for (int dy = 0; dy < 2; ++dy) {
                *j += (*i)( j.position().x() * f + dx, j.position().y() * f + dy, j.position().z() ) / 4;
            }
        if ( i == planes.begin() ) {
            DEBUG("Merged " << (*i)(0,0,0) << ", " << (*i)(0,1,0) << ", " << (*i)(1,0,0) << " and " << (*i)(1,1,0) << " to "
                  << t(0,0,0));
        }
        half_scale.push_back(t);
    }

    Derivator d( model, half_scale, scale_factor * f );
    Eigen::VectorXd scaled_guess = initial_guess;
    scaled_guess.head<2>() /= f;
    initial_guess = d.fit( initial_guess / f );
    initial_guess.head<2>() *= f;
    return initial_guess;
}

}
}
