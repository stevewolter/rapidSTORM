#include "debug.h"
#include "AndorDirect.h"
#include <dStorm/Image.h>
#include "LiveView.h"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace AndorCamera {

class SetFlagOnDestruction {
    ost::Mutex& m; 
    ost::Condition& c; 
    bool& f;
  public:
    SetFlagOnDestruction(ost::Mutex& m, ost::Condition& c, bool& flag) : m(m), c(c), f(f) {}
    ~SetFlagOnDestruction() {
        m.enterMutex();
        f = true;
        c.signal();
        m.leaveMutex();
    }
};

class Source::iterator {
  public:
    typedef std::input_iterator_tag iterator_category;
    typedef CamImage value_type;
    typedef ptrdiff_t difference_type;
    typedef CamImage* pointer;
    typedef CamImage& reference;

  private:
    Source* ad;
    boost::shared_ptr<CamImage> image;
    int next_image_number;

    void get_next_image();

  public:
    iterator() {}
    iterator(Source &ad);
    ~iterator() { ad->acquisition.stop(); }

    CamImage& operator*() { return *image; }
    const CamImage& operator*() const { return *image; }
    
    iterator& operator++() { get_next_image(); }
    iterator operator++(int) { iterator i = *this; ++(*this); return i; }

    bool operator==(const iterator& i)  const
        { return i.image.get() == image.get(); }
    bool operator!=(const iterator& i)  const
        { return i.image.get() != image.get(); }
};

Source::iterator::iterator(Source &ad)
: ad(&ad), next_image_number(0)
{ 
    SetFlagOnDestruction destruct(ad.initMutex, ad.is_initialized, ad.initialized);
    DEBUG("Started acquisition subthread");
    ad.acquisition.start();
    DEBUG("Waiting for acquisition to gain camera");
    ad.acquisition.block_until_on_camera();
    DEBUG("Acquisition gained camera");

    get_next_image();
}

void Source::iterator::get_next_image() {
    image.reset();
    while ( image.get() == NULL ) {
        try {
            CamImage::Size sz;
            sz.x() = ad->acquisition.getWidth() * cs_units::camera::pixel;
            sz.y() = ad->acquisition.getHeight() * cs_units::camera::pixel;
            image.reset( new CamImage(sz, next_image_number++ * cs_units::camera::frame) );
        } catch( const std::bad_alloc& alloc ) {
            /* Do nothing. Try to wait until more memory is available.
                * Maybe the ring buffer saves us. Maybe not, but we can't
                * do anything about that without memory. */
            continue;
        }
    }

    while ( true ) {
        AndorCamera::Acquisition::Fetch nextIm =
            ad->acquisition.getNextImage( image->ptr() );
        if ( nextIm.first == AndorCamera::Acquisition::NoMoreImages ) {
            image.reset();
            break;
        } else if ( nextIm.first != AndorCamera::Acquisition::HadError ) {
            ad->live_view->show( *image, nextIm.second );
            break;
        } else {
            continue;
        }
    }
}

CamSource::iterator
Source::begin() {
    return CamSource::iterator(iterator(*this));
}

CamSource::iterator
Source::end() {
    return CamSource::iterator(iterator());
}

}
