#include "debug.h"
#include "AndorDirect.h"
#include <dStorm/Image.h>
#include "LiveView.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace AndorCamera {

class SetFlagOnDestruction {
    ost::Mutex& m; 
    ost::Condition& c; 
    bool& f;
  public:
    SetFlagOnDestruction(ost::Mutex& m, ost::Condition& c, bool& flag) : m(m), c(c), f(flag) {}
    ~SetFlagOnDestruction() {
        m.enterMutex();
        f = true;
        c.signal();
        m.leaveMutex();
    }
};

class Source::iterator 
: public boost::iterator_facade<iterator,CamImage,std::input_iterator_tag>
{
  private:
    class CamRef;
    boost::shared_ptr<CamRef> ref;
    mutable CamImage img;

  public:
    iterator() {}
    iterator(Source &ad);

    CamImage& dereference() const;
    void increment();
    bool equal(const iterator& i) const {
        DEBUG("Comparing " << ref.get() << " and " << i.ref.get());
        return ref.get() == i.ref.get(); 
    }
};

struct Source::iterator::CamRef {
    Source &ad;
    int next_image_number;

    CamImage get_next_image();
    bool is_finished;

  public:
    CamRef(Source &ad);
    ~CamRef();
};

Source::iterator::iterator(Source &ad) 
    : ref(new CamRef(ad)) 
{
    increment();
}

CamImage& Source::iterator::dereference() const {
    return img;
}
void Source::iterator::increment() {
    img = ref->get_next_image();
    if ( ref->is_finished ) ref.reset();
}

Source::iterator::CamRef::CamRef(Source &ad)
: ad(ad), next_image_number(0), is_finished(false)
{ 
    SetFlagOnDestruction destruct(ad.initMutex, ad.is_initialized, ad.initialized);
    DEBUG("Started acquisition subthread");
    ad.acquisition.start();
    try {
        DEBUG("Waiting for acquisition to gain camera");
        ad.acquisition.block_until_on_camera();
        DEBUG("Acquisition gained camera");
    } catch (...) {
        DEBUG("Caught an error, stopping acquisition");
        ad.acquisition.stop();
        is_finished = true;
        throw;
    }
}

CamImage Source::iterator::CamRef::get_next_image() {
    CamImage image;

    DEBUG("Allocating image " << next_image_number);
    while ( image.is_invalid() ) {
        try {
            CamImage::Size sz;
            sz.x() = ad.acquisition.getWidth();
            sz.y() = ad.acquisition.getHeight();
            image = CamImage(sz, next_image_number * boost::units::camera::frame);
        } catch( const std::bad_alloc& alloc ) {
            /* Do nothing. Try to wait until more memory is available.
                * Maybe the ring buffer saves us. Maybe not, but we can't
                * do anything about that without memory. */
            continue;
        }
    }
    DEBUG("Allocated image");

    DEBUG("Fetching image " << next_image_number);
    while ( true ) {
        AndorCamera::Acquisition::Fetch nextIm =
            ad.acquisition.getNextImage( image.ptr() );
        if ( nextIm.first == AndorCamera::Acquisition::NoMoreImages ) {
            DEBUG("No more images");
            image.invalidate();
            is_finished = true;
            break;
        } else if ( nextIm.first != AndorCamera::Acquisition::HadError ) {
            DEBUG("Showing");
            ad.live_view->show( image, nextIm.second / boost::units::camera::frame );
            next_image_number += 1;
            break;
        } else {
            DEBUG("Error in fetching image");
            image.invalidate();
            next_image_number += 1;
            break;
        }
    }
    DEBUG("Fetched image");
    return image;
}

Source::iterator::CamRef::~CamRef() {
    DEBUG("Destructing camera reference");
    ad.acquisition.stop();
}

CamSource::iterator
Source::begin() {
    DEBUG("Beginning iteration for camera images");
    CamSource::iterator rv(iterator(*this));
    DEBUG("First image is " << rv->frame_number());
    return rv;
}

CamSource::iterator
Source::end() {
    return CamSource::iterator(iterator());
}

}
