#ifndef DSTORM_INPUT_SERIALSOURCE_IMPL_H
#define DSTORM_INPUT_SERIALSOURCE_IMPL_H

#include "SerialSource.h"

namespace dStorm {
namespace input {

template<typename Type>
Type*
SerialSource<Type>::fetch(int image_index)
{
    ost::MutexLock lock(serializer);
    if ( this->pushTarget != NULL ) {
        int serial = std::max(current_object+1, int(this->roi_start));
        for (int image = serial; image < image_index; image++)
        {
            Type* loaded = load();
            Management management;
            management = this->pushTarget->accept
                ( image - this->roi_start, 1, loaded );
            current_object = image;
            if ( management == Delete_objects )
                delete loaded;
        }
    }

    if ( current_object < image_index ) {
        assert( current_object == image_index - 1 );
        current_object = image_index;
        return load();
    } else
        return NULL;
}

}
}


#endif
