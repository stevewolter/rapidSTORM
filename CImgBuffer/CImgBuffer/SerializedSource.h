#ifndef CImgBuffer_IMAGESOURCES_H
#define CImgBuffer_IMAGESOURCES_H

#include <CImgBuffer/Source.h>
#include <CImgBuffer/Buffer.h>
#include <cc++/thread.h>
#include <set>
#include <memory>

namespace CImgBuffer {
    /** This specialized Source should be used as the base class for
     *  a Source that does neither allow concurrent accesses nor tolerates
     *  "jumps" in the input very well. A jump means successive calls
     *  to fetch() that do not refer to successive image indexes; for
     *  example, the sequence of fetch indices 2 3 4 5 contains no jumps,
     *  while 2 3 5 and 2 4 3 5 do.
     *
     *  The SerializedSource minimizes the occurence of such jumps by
     *  allowing only the lowest-numbered announced fetch to be executed.
     *  */
    template <typename T> 
    class SerializedSource 
        : public Source<T> 
    {
      protected:
        /** \var source This mutex locks the source object. */
        /** \var set    This mutex locks \c announced. */
        ost::Mutex source, set;
        /** A condition that is signaled if the front number of
         *  the queue might have changed. */
        ost::Condition queueFrontChange;
        /** An ordered list with all announced fetches. */
        std::multiset<int> announced;
        /** This method needs to be overriden for the implementation
         *  of a SerializedSource. It is guaranteed that only a single
         *  call to this method will happen concurrently and that the
         *  \a index arguments contain no unnecessary jumps. */
        virtual std::auto_ptr< T > get_image(int index)
            throw(std::exception) = 0;

      public:
        SerializedSource() throw();

        virtual std::auto_ptr< T > fetch(int index) throw(std::exception);
    };

}

#endif
