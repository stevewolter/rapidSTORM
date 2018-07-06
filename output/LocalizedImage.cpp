#include "output/LocalizedImage.h"
#include <iostream>

namespace dStorm {
namespace output {

LocalizedImage::LocalizedImage() {}

LocalizedImage::LocalizedImage(frame_index frame) : group(frame.value()) {}

LocalizedImage::LocalizedImage(const LocalizedImage& o) 
: results(o.results), group(o.group), source(o.source)
{
}

LocalizedImage::~LocalizedImage()
{
}

LocalizedImage& LocalizedImage::operator=( const LocalizedImage& o ) {
    group = o.group;
    results = o.results;
    source = o.source;
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
    group = n.value();
    for ( iterator i = begin(); i != end(); ++i )
        i->frame_number() = n;
}

}
}
