#define BOOST_DISABLE_ASSERTS
#include "impl.h"
#include "Derivator.h"
#include <boost/iterator/iterator_adaptor.hpp>
#include <dStorm/Image_iterator.h>
#include <dStorm/image/slice.h>
#include <dStorm/image/convert.h>
#include <dStorm/image/minmax.h>
#include <dStorm/helpers/DisplayDataSource_impl.h>
#include <Eigen/LU>
#include <functional>
#include <numeric>
#include <gsl/gsl_fit.h>

#include <dStorm/debug.h>

namespace locprec {
namespace biplane_alignment {

struct Source::_iterator
: public boost::iterator_adaptor<_iterator, Base::iterator>
{
    _iterator() : _iterator::iterator_adaptor_() {}
    _iterator(Base::iterator i, const MotionModel::Motion& m) : _iterator::iterator_adaptor_(i), m(m) {}

  private:
    MotionModel::Motion m;
    mutable dStorm::engine::Image img;

    friend class boost::iterator_core_access;
    void increment() { img.invalidate(); ++this->base_reference(); }
    dStorm::engine::Image& dereference() const {
        const dStorm::engine::Image::Size::Scalar lower = 1 * boost::units::camera::pixel;
        if ( img.is_invalid() ) {
            img = this->base()->deep_copy();
            dStorm::Image< dStorm::engine::Image::Pixel, 2 > lower_plane = img.slice(2,lower);
            Source::apply_motion( this->base()->slice(2,lower), m, lower_plane );
        }
        return img;
    }
};

Source::Source( const Config& c, std::auto_ptr< Base > base )
: Object("BiplaneAlignment", "Biplane alignment"),
  Base(*this, base->flags),
  base(base), model( c.model().clone() )
{
    model->parameter_count();
}

Source::Base::iterator Source::begin() { return Base::iterator(_iterator(base->begin(), motion)); }
Source::Base::iterator Source::end() { return Base::iterator(_iterator(base->end(), motion)); }

class Source::Whitening 
{
    double a, I0;
  public:
    Whitening( double incline, double shift ) 
        : a(incline), I0( shift / (1-incline) ) 
        { DEBUG("Estimated ratio at " << a << " and I0 at " << I0); }
    float operator()( unsigned short uw ) { return (uw - I0) / a + I0; }
};

Source::Whitening Source::get_whitening_factors()
{
    static const int max_depth = 2, max_images = 1000;
    Eigen::Matrix<double, max_images, max_depth> sums;
    int c = 0, pixel_count = 0;
    double c0 = 0, c1 = 1.0001, cov00, cov01, cov11, residues;
    for ( Source::Base::iterator i = base->begin(); i != base->end(); ++i, ++c )
    {
        for (int j = 0; j < max_depth; ++j) {
            dStorm::Image<unsigned short,2> s = i->slice( 2, j * boost::units::camera::pixel );
            assert( pixel_count == 0 || pixel_count == int(s.size_in_pixels()) );
            pixel_count = s.size_in_pixels();
            if ( j < i->depth_in_pixels() ) 
                sums(c, j) = std::accumulate( s.begin(), s.end(), 0.0f );
            else
                sums(c, j) = 0;
        }

        if ( c % 10 == 1 ) {
            int rv = gsl_fit_linear( sums.data(), 1, sums.col(1).data(), 1, c+1, &c0, &c1, &cov00, &cov01, &cov11, &residues );
            if ( rv != GSL_SUCCESS )
                throw std::runtime_error( gsl_strerror(rv) );
            if ( sqrt(cov00) <= 0.05 * c0 && sqrt(cov11) <= 0.05 * c1 )
                break;
            else {
                DEBUG("Standard deviations " << sqrt(cov00) << " " << sqrt(cov11) << " is too high for " << c1 << " " << c0 << ", continue searching whitening factor");
            }
        }
    }
    if ( c < 2 ) 
        return Whitening( sums(0,0) / sums(0,1), 0 );
    else
        return Whitening( c1, c0 / pixel_count );
}

template <typename Base>
struct adder : public Base {
    adder( const Base& b ) : Base(b) {}
    struct plus_assigner {
        Base& b; plus_assigner(Base& b) : b(b) {}
        template <typename Type>
        plus_assigner& operator=( const Type& rvalue ) { *b += rvalue; return *this; }
    };
    plus_assigner operator*() { return plus_assigner(*this); }
};

template <typename Base>
adder<Base> make_adder( const Base& b ) { return adder<Base>(b); }

Source::Base::TraitsPtr Source::get_traits()
{
    using boost::units::camera::pixel;
    Source::Base::TraitsPtr rv = base->get_traits();
    std::vector< Image > image_data;

    Whitening whitening = get_whitening_factors();
    base->dispatch( BaseSource::RepeatInput );
    for ( Source::Base::iterator i = base->begin(); i != base->end(); ++i )
    {
        Image my_copy( i->sizes() );
        Plane keep = my_copy.slice(2, 0 * pixel);
        std::copy( i->slice(2, 0 * pixel).begin(), i->slice(2, 0 * pixel).end(), keep.begin() );
        for (int j = 1; j < my_copy.depth_in_pixels(); ++j ) {
            dStorm::Image<dStorm::engine::Image::Pixel,2> orig = i->slice(2,j*pixel);
            Plane to_whiten = my_copy.slice(2, j * pixel);
            DEBUG("Unwhitened planes have means " << std::accumulate( orig.begin(), orig.end(), 0.0 ) << " and " << 
                std::accumulate( keep.begin(), keep.end(), 0.0 ) );
            std::transform( orig.begin(), orig.end(), to_whiten.begin(), whitening);
            DEBUG("Whitened planes have means " << std::accumulate( to_whiten.begin(), to_whiten.end(), 0.0 ) << " and " << 
                std::accumulate( keep.begin(), keep.end(), 0.0 ) );
        }
        for (int acc = 1; acc < 10 && i != base->end(); ++i, ++acc) {
            std::copy( i->slice(2, 0 * pixel).begin(), i->slice(2, 0 * pixel).end(), make_adder( keep.begin() ) );
            for (int j = 1; j < my_copy.depth_in_pixels(); ++j ) {
                dStorm::Image<dStorm::engine::Image::Pixel,2> orig = i->slice(2,j*pixel);
                Plane to_whiten = my_copy.slice(2, j * pixel);
                std::transform( orig.begin(), orig.end(), make_adder( to_whiten.begin() ), whitening);
            }
        }
        image_data.push_back( my_copy );
        if ( image_data.size() >= 1 ) break;
    }
    base->dispatch( BaseSource::RepeatInput );

    Derivator d( *model, image_data );
    Eigen::VectorXd parameters( model->parameter_count(), 1 );
    parameters.fill(0);
    parameters = d.fit(parameters);
    motion = model->get_motion( parameters );
    std::cerr << "Final motion model " << motion << std::endl;
    DEBUG("Motion model is\n" << motion);

    return rv;
}

#undef DEBUG
#undef SIMPARM_DEBUG
#include <dStorm/debug.h>

template <typename Pixel>
void Source::apply_motion( const dStorm::Image<Pixel,2>& img, const MotionModel::Motion& m, dStorm::Image<Pixel,2>& target )
{
    DEBUG("Scaling image with motion model " << m);
    Eigen::Matrix3d inverse_motion = m.inverse();
    for ( typename dStorm::Image<Pixel,2>::iterator i = target.begin(); i != target.end(); ++i ) 
    {
        Eigen::Vector3d in_target_space; 
        in_target_space.x() = i.position().x();
        in_target_space.y() = i.position().y();
        in_target_space.z() = 1;
        
        Eigen::Vector2d src = (inverse_motion * in_target_space).head<2>();
        DEBUG("Position in source space for " << in_target_space.head<2>().transpose() << " is " << src.transpose());
        /* Linear interpolation */
        Eigen::Vector2f x_dist, y_dist;
        int xb = floor(src.x()), yb = floor(src.y());
        x_dist[0] = 1 - (src.x() - xb); x_dist[1] = 1 - x_dist[0];
        y_dist[0] = 1 - (src.y() - yb); y_dist[1] = 1 - y_dist[0];
        Eigen::Matrix2f weights = x_dist * y_dist.transpose();

        float value = 0;
        for (int dx = 0; dx < 2; ++dx) for (int dy = 0; dy < 2; ++dy) 
        {
            DEBUG("Adding " << xb+dx << "," << yb+dy << " with weight " << weights(dx,dy) );
            value += weights(dx,dy) * img( 
                std::max(0, std::min(xb+dx, img.width_in_pixels()-1)),
                std::max(0, std::min(yb+dy, img.height_in_pixels()-1)) );
        }
        *i = round(value);
    }
}

template void Source::apply_motion<unsigned short>( const dStorm::Image<unsigned short,2>&, const MotionModel::Motion&, dStorm::Image<unsigned short,2>& );
template void Source::apply_motion<float>( const dStorm::Image<float,2>&, const MotionModel::Motion&, dStorm::Image<float,2>& );

}
}
