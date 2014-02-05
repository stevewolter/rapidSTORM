#include "dStorm/output/LocalizedImage.h"
#include <iostream>

namespace dStorm {
namespace output {

LocalizedImage::LocalizedImage() 
: forImage(0 * camera::frame), candidates(NULL)
{
}

LocalizedImage::LocalizedImage(frame_index i) 
: forImage(i), candidates(NULL)
{
}

LocalizedImage::LocalizedImage(const LocalizedImage& o) 
: results(o.results), forImage(o.forImage), source(o.source), smoothed(o.smoothed), 
  candidates(o.candidates)
{
}

LocalizedImage::~LocalizedImage()
{
}

LocalizedImage& LocalizedImage::operator=( const LocalizedImage& o ) {
    forImage = o.forImage;
    results = o.results;
    source = o.source;
    smoothed = o.smoothed;
    candidates = o.candidates;
    return *this;
}

void LocalizedImage::push_back( const Localization& l ) { results.push_back(l); }
void LocalizedImage::clear() { results.clear(); }
void LocalizedImage::resize( size_t size ) { results.resize( size ); }

bool LocalizedImage::empty() const { return results.empty(); }

LocalizedImage::iterator LocalizedImage::erase( iterator p ) 
    { return results.erase(p); }
LocalizedImage::iterator LocalizedImage::erase( iterator f, iterator t ) 
    { return results.erase(f,t); }

void LocalizedImage::set_frame_number( frame_index n ) 
{
    forImage = n;
    for ( iterator i = begin(); i != end(); ++i )
        i->frame_number() = n;
}

}
}
