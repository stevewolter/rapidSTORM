#ifndef DSTORM_INPUT_DRAIN_H
#define DSTORM_INPUT_DRAIN_H

#include <any_iterator.hpp>

namespace dStorm { 
namespace input { 

    enum Management { Keeps_objects, Delete_objects };

    /** A Drain is a base class for all objects that accept images
     *  from a pushing Source. */
    template <class T> class Drain {
      public:
        typedef IteratorTypeErasure::any_iterator< T, std::output_iterator_tag > iterator;

        virtual iterator begin() = 0;
    };
}
}

#endif
