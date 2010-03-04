#ifndef DATA_CPP_VECTORLIST_H
#define DATA_CPP_VECTORLIST_H

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cassert>
#include <stdexcept>

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
     *  new elements are filled into the last bin.
     *
     *  The Alignment template parameter can be choosen to
     *  any value != 1 to ensure values are aligned at these
     *  boundaries. */
    template <typename Type, unsigned int Alignment = 0x1>
    class VectorList {
      private:
        Type **array, **allocated;
        const int binSize;
        int arrayBins, currentBin, indexInBin;

        void makeAlignedBin(int size) {
            void *fresh = malloc(size+(Alignment-1));
            if ( fresh == NULL ) throw std::bad_alloc();
            size_t addr = reinterpret_cast<size_t>(fresh) & ~(Alignment-1);
            if ( addr != reinterpret_cast<size_t>(fresh) )
                addr += Alignment;
            void *aligned = reinterpret_cast<void*>(addr);

            allocated[currentBin] = (Type*)fresh;
            array[currentBin] = (Type*)aligned;
        }
        void init() {
            size_t size = sizeof(Type*) * arrayBins;
            array = (Type**)malloc(size);
            allocated = (Type**)malloc(size);
            if ( array == NULL || allocated == NULL )
                throw std::bad_alloc();

            currentBin = indexInBin = 0;

            makeAlignedBin( sizeof(Type) * binSize );
            for (int i = 1; i < arrayBins; i++)
                array[i] = NULL;
        }
        static void enlarge(Type**& array, int old_size, int new_size) {
            Type **newArray = (Type**)malloc
                (sizeof(Type*) * new_size);
            if ( newArray == NULL ) throw std::bad_alloc();
            memcpy(newArray, array, sizeof(Type*) * old_size);
            memset(newArray+old_size, 0, 
                             sizeof(Type*) * (new_size-old_size) );
            free(array);
            array = newArray;
        }
        void makeNewBin() {
            currentBin++;
            if (currentBin == arrayBins) {
                int newArrayBins = 2 * arrayBins;
                enlarge(array, arrayBins, newArrayBins);
                enlarge(allocated, arrayBins, newArrayBins);
                arrayBins = newArrayBins;
            }

            if (array[currentBin] == NULL)
                makeAlignedBin( sizeof(Type) * binSize );
            
            indexInBin = 0;
        }
        void dealloc() {
            for (int i = 0; i < arrayBins; i++)
                if ( array[i] != NULL ) {
                    for (int e = 0; e < sizeOfBin(i); e++) {
                        array[i][e].~Type();
                    }
                    free(allocated[i]);
                    array[i] = NULL;
                } else
                    break;

            if ( array ) free(array);
            array = NULL;
            if ( allocated ) free(allocated);
            allocated = NULL;
        }

      public:
        /** Constructs an empty VectorList */
        VectorList() : binSize(1000), arrayBins(1000) { init(); }
        /** Destructor. */
        ~VectorList() { dealloc(); }

        VectorList (const VectorList<Type>& copy_from) 
        : binSize(copy_from.binSize)
        {
            arrayBins = 0; array = NULL; allocated = NULL;
            (*this) = copy_from;
        }
        VectorList<Type>& operator=(const VectorList<Type>& from)
        {
            if ( this == &from ) return *this;
            if ( arrayBins < from.currentBin ) {
                dealloc();
                arrayBins = from.arrayBins;
                init();
            } else if ( arrayBins == 0 ) {
                arrayBins = 1000;
                init();
            }
            while ( currentBin < from.currentBin )
                makeNewBin();

            for (int bin = 0; bin < from.binNumber(); bin++)
                for (int el = 0; el < from.sizeOfBin(bin); el++)
                {
                    new (array[bin]+el) Type(from[bin*binSize+el]);
                }

            currentBin = from.currentBin;
            indexInBin = from.indexInBin;
            return *this;
        }


        inline const Type& operator[](int index) const {
            assert( index <= currentBin * binSize + indexInBin );
            return array[ index / binSize ][ index % binSize ];
        }
        /** Returns the number of independent bins that could 
         *  be accessed with getBin(). */
        inline int binNumber() const { return currentBin+1; }
        /** Get a pointer to the elements in bin number \c num. */
        inline const Type* getBin(int num) const
            { return array[num]; }
        /** Get a pointer to the elements in bin number \c num. */
        inline Type* getBin(int num) { return array[num]; }
        /** Get the number of elements in the bin number \c num. */
        inline int sizeOfBin(int num) const 
            { return (num < currentBin) ? binSize : (num == currentBin) ? indexInBin : 0; }

        /** Add a new element to the VectorList. */
        inline void push_back(const Type &fit) { 
            Type *mem = allocate();
            new(mem) Type(fit);
        } 

        /** Allocate space for a new element, but do not initialize. */
        inline Type* allocate() { 
            if (indexInBin == binSize) 
                makeNewBin();
            Type* rv = array[currentBin] + indexInBin;
            indexInBin++;
            return rv;
        }

        /** Returns the total number of elements in the VectorList. */
        inline unsigned int size() const 
            { return binSize * currentBin + indexInBin; }
         
        /** Removes all elements from the VectorList. */
        inline void clear()
            { currentBin = indexInBin = 0; }
         
        /** Returns a reference to the first element in the
         *  VectorList. */
        inline Type& front()
            { return array[0][0]; }
        /** Returns a reference to the last element in the
         *  VectorList. */
        inline Type& back()
            { if (indexInBin == 0)
                return array[currentBin-1][binSize-1];
              else
                return array[currentBin][indexInBin-1]; 
            }

        /** Standard STL iterator. */
      private:
        template <typename RetType>
        class _iterator {
          private:
            friend class VectorList;
            RetType * const *array;
            int run, binSize;

            _iterator(int binSize, RetType * const *array,
                      int curEl)
                : array(array),
                  run(curEl), binSize(binSize){}

          public:
            operator _iterator<const RetType>() const
                { return _iterator<const RetType>(binSize, array, run); }

            inline RetType &operator*() const
                { return (*array)[run]; }
            inline RetType *operator->() const
                { return (*array)+run; }

            inline bool operator!=(const _iterator &other)
                { return ! ((*this) == other); }
            inline bool operator==(const _iterator &other)
                { return array == other.array && run == other.run; }

            inline _iterator& operator++() { 
                run++;
                if (run == binSize) { ++array; run = 0; }
                return *this;
            }

            inline _iterator operator++(int) 
                { _iterator tmp(*this); ++(*this); return tmp; }
        };
        friend class _iterator<const Type>;
        friend class _iterator<Type>;
      public:
        typedef _iterator<const Type> const_iterator;
        typedef _iterator<Type> iterator;

        /** Returns an iterator to the first element. */
        inline const_iterator begin() const
            { return const_iterator(binSize, array, 0); }
        inline iterator begin()
            { return iterator(binSize, array, 0); }

        /** Returns an iterator past the last element. */
        inline const_iterator end() const
        { 
            int cB = currentBin + indexInBin / binSize, 
                 i = indexInBin % binSize;
            return const_iterator(binSize, array+cB, i);
        }
        /** Returns an iterator past the last element. */
        inline iterator end() 
        { 
            int cB = currentBin + indexInBin / binSize, 
                 i = indexInBin % binSize;
            return iterator(binSize, array+cB, i);
        }

    };
}

#endif
