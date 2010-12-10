#include <CImgBuffer/SerializedSource.h>
#include <foreach.h>
#include <algorithm>
#include <iostream>
#include <CImgBuffer/Image.h>

#include "Pixel.h"

using namespace std;

namespace CImgBuffer {

template<>
SerializedSource< Image<Pixel> >::SerializedSource() throw() 
: queueFrontChange(set)
{ 
}

template<>
std::auto_ptr< Image<Pixel> > 
SerializedSource< Image<Pixel> >::fetch(int index) 
throw(std::exception) 
{
    PROGRESS("fetch() in SerializedSource");
    set.enterMutex();
    announced.insert( index );
    while ( index != *min_element(announced.begin(), announced.end()) ) {
        LOCKING("Waiting for " << index << " to get queue front " <<
                *min_element(announced.begin(), announced.end()));
        queueFrontChange.wait();
        LOCKING("Finished queue front wait.");
    }
    PROGRESS("Got queue front");

    announced.erase( announced.find(index) );
    queueFrontChange.signal();

    ost::MutexLock lock(source);
    set.leaveMutex();

    PROGRESS("Fetching image");
    std::auto_ptr< Image<Pixel> > rv = get_image(index);
    PROGRESS("Got image");
    return rv;
}

}
