#ifndef CIMGBUFFER_PROPERTIES_H
#define CIMGBUFFER_PROPERTIES_H

namespace CImgBuffer {
/** The Traits class confers information about objects in a Source.
 *  It can be specialized for concrete objects; in that case, make
 *  sure all users of the Source class see the relevant specialization. */
template <typename Type> 
class Traits {};
}

#endif
