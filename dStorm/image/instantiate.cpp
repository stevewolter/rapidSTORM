#include "dStorm/Image.h"
#include "dStorm/image/constructors.h"

namespace dStorm {

#define INSTANTIATIONS(t,n) \
    template Image<t,n>::Image(); \
    template Image<t,n>::~Image(); \
    template Image<t,n>::Image( Image<t,n>::Size, frame_index );

INSTANTIATIONS(unsigned short,2);
INSTANTIATIONS(unsigned int,2);
INSTANTIATIONS(unsigned short,3);
INSTANTIATIONS(unsigned int,3);

}
