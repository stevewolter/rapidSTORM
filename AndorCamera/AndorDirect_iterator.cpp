#include "debug.h"
#include "AndorDirect.h"
#include "CameraConnection.h"
#include <dStorm/Image.h>
#include "LiveView.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

namespace dStorm {
namespace AndorCamera {

class Source::iterator 
: public boost::iterator_facade<iterator,CamImage,std::input_iterator_tag>,
  public boost::static_visitor<void>
{
  private:
    Source* src;
    mutable CamImage img;

  public:
    iterator() : src(NULL) {}
    iterator(Source &ad);

    CamImage& dereference() const;
    void increment();
    bool equal(const iterator& i) const { return src == i.src; }

    void operator()( const CameraConnection::FetchImage& );
    void operator()( const CameraConnection::ImageError& );
    void operator()( const CameraConnection::EndOfAcquisition& );
};

Source::iterator::iterator(Source &ad) 
    : src(&ad)
{
    increment();
}

CamImage& Source::iterator::dereference() const {
    return img;
}


void Source::iterator::increment() {
    if ( !src ) return;

    boost::lock_guard<boost::mutex> guard(src->mutex);
    if ( src->has_ended ) 
        src = NULL;
    else {
        CameraConnection::FrameFetch f = src->connection->next_frame();
        boost::apply_visitor(*this, f);
    }
}

void Source::iterator::operator()( const CameraConnection::FetchImage& fr )
{
    DEBUG("Allocating image " << next_image_number);
    while ( img.is_invalid() ) {
        try {
            img = CamImage(src->traits->size, fr.frame_number);
        } catch( const std::bad_alloc& alloc ) {
            /* Do nothing. Try to wait until more memory is available.
                * Maybe the ring buffer saves us. Maybe not, but we can't
                * do anything about that without memory. */
            continue;
        }
    }
    DEBUG("Allocated image");
    src->connection->read_data( img );
    if ( src->live_view.get() )
        src->live_view->show( img );
}

void Source::iterator::operator()( const CameraConnection::ImageError& fr )
{
    img.invalidate();
    img.frame_number() = fr.frame_number;
}

void Source::iterator::operator()( const CameraConnection::EndOfAcquisition& fr )
{
    img.invalidate();
    src->has_ended = true;
    src = NULL;
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
}
