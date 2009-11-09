#ifndef DSTORM_INPUT_DRAIN_H
#define DSTORM_INPUT_DRAIN_H

namespace dStorm { 
namespace input { 

    enum Management { Keeps_objects, Delete_objects };

    /** A Drain is a base class for all objects that accept images
     *  from a pushing Source. */
    template <class T> class Drain {
      public:
        /** Accept an image. 
         *  \param first_index Index as labeled by source, numbered
         *               from 0.
         *  \param number Number of transferred objects.
         *  \param i     The object for that index. If this pointer
         *               is NULL, an error occurred and these objects
         *               will not be transferred.
         *  \return      Whether the pushing code may delete the pushed
         *               objects. */
        virtual Management 
            accept(int first_index, int number, T* i) = 0;

        /** Receive the number of objects that will be pushed. */
        virtual void receive_number_of_objects(int) {}
    };
}
}

#endif
