#include "debug.h"
#include "CameraConnection.h"
#include "AndorDirect.h"
#include <dStorm/Image.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/slice.h>
#include "LiveView.h"
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>

namespace dStorm {
namespace AndorCamera {

class Source::iterator 
: public boost::iterator_facade<iterator,engine::Image,std::input_iterator_tag>,
  public boost::static_visitor<void>
{
  private:
    Source* src;
    mutable engine::Image img;

  public:
    iterator() : src(NULL) {}
    iterator(Source &ad);

    engine::Image& dereference() const;
    void increment();
    bool equal(const iterator& i) const { return src == i.src; }

    inline void operator()( const CameraConnection::FetchImage& );
    inline void operator()( const CameraConnection::ImageError& );
    inline void operator()( const CameraConnection::EndOfAcquisition& );
    inline void operator()( const CameraConnection::Simparm& );
    inline void operator()( const CameraConnection::StatusChange& );
};

Source::iterator::iterator(Source &ad) 
    : src(&ad)
{
    increment();
}

engine::Image& Source::iterator::dereference() const {
    return img;
}


void Source::iterator::operator()( const CameraConnection::FetchImage& fr )
{
    DEBUG("Allocating image " << fr.frame_number);
    img.invalidate();
    while ( img.is_invalid() ) {
        try {
            img = dStorm::engine::Image(src->traits->size, fr.frame_number);
        } catch( const std::bad_alloc& alloc ) {
            /* Do nothing. Try to wait until more memory is available.
                * Maybe the ring buffer saves us. Maybe not, but we can't
                * do anything about that without memory. */
            continue;
        }
    }
    CamImage i(img.slice(2, 0));
    i.frame_number() = fr.frame_number;
    DEBUG("Allocated image");
    src->connection->read_data( i );
    if ( src->live_view.get() )
        src->live_view->show( i );
}

void Source::iterator::operator()( const CameraConnection::ImageError& fr )
{
    DEBUG("Image error for number " << fr.frame_number);
    img.invalidate();
    img.frame_number() = fr.frame_number;
}

void Source::iterator::operator()( const CameraConnection::EndOfAcquisition& )
{
    DEBUG("Acquisition has ended");
    img.invalidate();
    src->has_ended = true;
    src = NULL;
}

void Source::iterator::operator()( const CameraConnection::Simparm& )
{
    DEBUG("Acquisition got simparm message ");
}

void Source::iterator::operator()( const CameraConnection::StatusChange& status ) {
    DEBUG("Acquisition got status message " << status.status);
    src->status = status.status;
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
