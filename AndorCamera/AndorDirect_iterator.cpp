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
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace AndorCamera {

class Source::iterator 
: public boost::iterator_facade<iterator,engine::ImageStack,std::input_iterator_tag>,
  public boost::static_visitor<void>
{
  private:
    Source* src;
    mutable boost::optional<engine::ImageStack> img;
    bool image_ready;

  public:
    iterator() : src(NULL), image_ready(false) {}
    iterator(Source &ad);

    engine::ImageStack& dereference() const;
    void increment();
    bool equal(const iterator& i) const { return src == i.src; }

    inline void operator()( const CameraConnection::FetchImage& );
    inline void operator()( const CameraConnection::ImageError& );
    inline void operator()( const CameraConnection::EndOfAcquisition& );
    inline void operator()( const CameraConnection::Simparm& );
    inline void operator()( const CameraConnection::StatusChange& );
};

Source::iterator::iterator(Source &ad) 
    : src(&ad), image_ready(false)
{
    increment();
}

engine::ImageStack& Source::iterator::dereference() const {
    return *img;
}


void Source::iterator::operator()( const CameraConnection::FetchImage& fr )
{
    DEBUG("Allocating image " << fr.frame_number);
    img.reset();
    while ( ! img.is_initialized() ) {
        try {
            img = dStorm::engine::ImageStack(fr.frame_number);
            for (int p = 0; p < src->traits->plane_count(); ++p)
                img->push_back( dStorm::engine::Image2D( src->traits->image(p).size ) );
        } catch( const std::bad_alloc& alloc ) {
            /* Do nothing. Try to wait until more memory is available.
                * Maybe the ring buffer saves us. Maybe not, but we can't
                * do anything about that without memory. */
            continue;
        }
    }
    CamImage i(img->plane(0));
    i.frame_number() = fr.frame_number;
    DEBUG("Reading image " << fr.frame_number);
    src->connection->read_data( i );
    if ( src->live_view.get() )
        src->live_view->show( i );
    image_ready = true;
}

void Source::iterator::operator()( const CameraConnection::ImageError& fr )
{
    DEBUG("Image error for number " << fr.frame_number);
    img = dStorm::engine::ImageStack(fr.frame_number);
    img->push_back( dStorm::engine::Image2D() );
    image_ready = true;
}

void Source::iterator::operator()( const CameraConnection::EndOfAcquisition& )
{
    DEBUG("Acquisition has ended");
    img.reset();
    src->has_ended = true;
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

    while ( ! image_ready ) {
        DEBUG("Waiting for frame in camera connection");
        CameraConnection::FrameFetch f = src->connection->next_frame();
        boost::apply_visitor(*this, f);
        if ( src->has_ended ) {
            DEBUG("Got end of file");
            src = NULL;
            break;
        }
    }
    image_ready = false;
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
