#ifndef DATA_CPP_VECTORLIST_H
#define DATA_CPP_VECTORLIST_H

#include <stdlib.h>
#include <string.h>

namespace data_cpp {
    /** A VectorList is similar to the Java ArrayList:
     *  It is a data structure designed for random access
     *  and appending data.
     *
     *  In this implementation, it is a vector of vectors.
     *  The secondary vectors have a fixed size \c binSize.
     *  All allocation is done using malloc().
     *
     *  The secondary vectors are called bins. At any time,
     *  all bins but the last are filled completely, while
     *  new elements are filled into the last bin. */
    template <typename T> class VectorList {
      private:
        T **array;
        const int binSize;
        int arrayBins, currentBin, indexInBin;

        void init() throw() {
            arrayBins = 100;
            size_t size = sizeof(T*) * arrayBins;
            array = (T**)malloc(size);
            array[0] = (T*)malloc(sizeof(T) * binSize);
            for (int i = 1; i < arrayBins; i++)
                array[i] = NULL;

            currentBin = indexInBin = 0;
        }
        void makeNewBin() throw() {
            currentBin++;
            if (currentBin == arrayBins) {
                int newArrayBins = 2 * arrayBins;
                T **newArray = (T**)malloc
                    (sizeof(T*) * newArrayBins);
                memcpy(newArray, array, sizeof(T*) * arrayBins);
                memset(newArray+arrayBins, 0, sizeof(T*) * arrayBins);
                arrayBins = newArrayBins;
                free(array);
                array = newArray;
            }

            if (array[currentBin] == NULL) {
                array[currentBin] = (T*)malloc
                    (sizeof(T) * binSize);
            }
            
            indexInBin = 0;
        }
        void dealloc() throw() {
            for (int i = 0; i < arrayBins; i++)
                if ( array[i] != NULL ) {
                    free(array[i]);
                    array[i] = NULL;
                } else
                    break;
        }

      public:
        /** Constructs an empty VectorList */
        VectorList() throw() : binSize(1000) { init(); }
        /** Destructor. */
        ~VectorList() throw() { dealloc(); }

        /** Returns the number of independent bins that could 
         *  be accessed with getBin(). */
        inline int binNumber() const throw() { return currentBin+1; }
        /** Get a pointer to the elements in bin number \c num. */
        inline const T* getBin(int num) const throw()
            { return array[num]; }
        /** Get a pointer to the elements in bin number \c num. */
        inline T* getBin(int num) throw() { return array[num]; }
        /** Get the number of elements in the bin number \c num. */
        inline int sizeOfBin(int num) const throw() 
            { if (num < currentBin) return binSize; 
              else return indexInBin; }

        /** Add a new element to the VectorList. */
        inline void push_back(const T &fit) throw() { 
            new(allocate()) T(fit);
        } 

        /** Allocate space for a new element, but do not initialize. */
        inline T* allocate() throw() { 
            if (indexInBin == binSize) 
                makeNewBin();
            T* rv = array[currentBin] + indexInBin;
            indexInBin++;
            return rv;
        }

        /** Returns the total number of elements in the VectorList. */
        inline unsigned int size() throw() 
            { return binSize * currentBin + indexInBin; }
         
        /** Removes all elements from the VectorList. */
        inline void clear() throw()
            { currentBin = indexInBin = 0; }
         
        /** Returns a reference to the first element in the
         *  VectorList. */
        inline T& front() throw()
            { return array[0][0]; }
        /** Returns a reference to the last element in the
         *  VectorList. */
        inline T& back() throw()
            { if (indexInBin == 0)
                return array[currentBin-1][binSize-1];
              else
                return array[currentBin][indexInBin-1]; 
            }

        /** Standard STL iterator. */
        class const_iterator {
          private:
            friend class VectorList;
            T **array;
            T *run;
            int runCount, binSize;

          public:
            const_iterator(int binSize) throw() : binSize(binSize) {}
            inline const T &operator*() throw() { return *run; }
            inline const T *operator->() throw() { return run; }

            inline bool operator!=(const const_iterator &other) throw()
                { return ! ((*this) == other); }
            inline bool operator==(const const_iterator &other) throw()
                { return array == other.array && run == other.run; }

            inline const_iterator& operator++() throw() 
                { return operator++(0); }

            inline const_iterator& operator++(int) throw()
                { run++; runCount++; 
                  if (runCount == binSize) 
                    { run = *( ++array ); runCount = 0; }
                  return *this;
                }

        };
        friend class const_iterator;

        /** Returns an iterator to the first element. */
        inline const_iterator begin() const throw()
        { 
            const_iterator it(binSize);
            it.array = array;
            it.run = array[0];
            it.runCount = 0;
            return it;
        }

        /** Returns an iterator past the last element. */
        inline const_iterator end() const throw()
        { 
            const_iterator it(binSize);
            int cB = currentBin + indexInBin / binSize, i = indexInBin % binSize;
            it.array = array + cB;
            it.run = array[cB] + i;
            it.runCount = i;
            return it;
        }

    };
}

#endif
