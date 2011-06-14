#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdexcept>
#include "Mask.h"
#ifdef HAVE_LIBGRAPHICSMAGICK__
#include <Magick++.h>
#endif

#include <dStorm/engine/Image.h>
#include <dStorm/input/Source_impl.h>

namespace dStorm {
namespace input {

template <typename Ty>
class Mask<Ty>::_iterator 
  : public boost::iterator_adaptor< 
        Mask<Ty>::_iterator,
        typename Source<Ty>::iterator >
{
    const std::vector<bool>& mask;
    mutable bool masked;

    void mask_image() const {
        if ( ! this->base()->is_invalid() ) {
            unsigned long length = this->base()->size_in_pixels();
            if ( length > mask.size() )
                throw std::runtime_error("Mask image is too small");
            for ( unsigned int i = 0; i < length; ++i ) {
                if ( ! mask[i] ) (*this->base())[i] = 0;
            }
        }
        masked = true;
    }
 public:
    explicit _iterator(const iterator& from, const std::vector<bool>& mask)
      : _iterator::iterator_adaptor_(from), mask(mask) {}

 private:
    friend class boost::iterator_core_access;
    void increment() { 
        ++this->base_reference();
        masked = false;
    }

    typename _iterator::iterator_adaptor_::reference dereference() const {
        typename _iterator::iterator_adaptor_::reference r = *this->base();
        if ( ! masked ) mask_image();
        return r;
    }
};

template <typename Ty>
Mask<Ty>::Mask(std::auto_ptr< Source<Ty> > backend, const MaskConfig& config)
: Source<Ty>(backend->getNode(), backend->flags),
  s(backend)
{
    if ( !config.mask_image ) throw std::logic_error("No mask image given, but tried to apply mask filter");

#ifndef HAVE_LIBGRAPHICSMAGICK__
    throw std::logic_error("rapidSTORM was compiled without mask image support, sorry.");
#else
    Magick::Image mask_image( config.mask_image() );
    mask.resize( mask_image.rows() * mask_image.columns() );
    for (unsigned int y = 0; y < mask_image.rows(); ++y) {
        const Magick::PixelPacket* pixels = mask_image.getConstPixels(0, y, mask_image.columns(), 1);
        for (unsigned int x = 0; x < mask_image.columns(); ++x) {
            mask[y * mask_image.columns() + x] = ( pixels[x].red > 0 || pixels[x].green > 0 || pixels[x].blue > 0 );
        }
    }
#endif
    
}

template <typename Ty>
typename Mask<Ty>::iterator
Mask<Ty>::begin() { return iterator(_iterator(s->begin(), mask)); }

template <typename Ty>
typename Mask<Ty>::iterator
Mask<Ty>::end() { return iterator(_iterator(s->end(), mask)); }

template class Mask<dStorm::engine::Image>;

}
}
