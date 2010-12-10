#ifndef DSTORM_SMALLVECTOR_H
#define DSTORM_SMALLVECTOR_H

#include <stdlib.h>
#include <string.h>
#include <iostream>

namespace data_cpp {

/** The Vector class allows storage for a single data vector.
 *  It allocates sufficient storage to hold all data elements
 *  and provides direct access to elements or a C vector con-
 *  taining them.
 *
 *  The Vector class uses malloc() for storage allocation. If
 *  the malloc() implementation is thread-safe, concurrent
 *  accesses to different vectors are safe as well.
 */
template <typename T> class Vector {
  protected:
    T *data;
    int length, alloc;
    
    inline void do_realloc() throw() {
        T *new_data = (T*)malloc(sizeof(T) * alloc);
        if (new_data == NULL) 
            { std::cerr << "Memory allocation for vector of size "
                        << alloc << " failed." << std::endl;
              exit(1); }
        if ( data != NULL ) {
            memcpy( new_data, data, sizeof(T) * length );
            free(data);
        }
        data = new_data;
    }
    inline void realloc_if_necessary() throw() {
        if (length == alloc) {
            alloc *= 2;
            do_realloc();
        }
    }

  public:
    /** Construct an empty vector with a capacity of 10. */
    Vector () throw ()
    : data(NULL), length(0), alloc(10) 
    { do_realloc(); }
    /** Construct an empty vector with a size and capacity of \c initialSize. */
    Vector (int initialSize) throw ()
    : data(NULL), length(initialSize), alloc(initialSize) 
    { do_realloc();
      for (int i = 0; i < length; i++) new(data+i) T(); }
    /** Construct an empty vector with a size and capacity of \c initialSize
     *  and copy its values from init_val. */
    Vector (int initial_size, const T& init_val) throw ()
    : data(NULL), length(initial_size), alloc(initial_size) 
    { do_realloc();
      for (int i = 0; i < length; i++) new(data+i) T(init_val); }
    /** Construct an empty vector with a size of \c initial_size and capacity
     *  of \c initial_allocation and copy its values from init_val. */
    Vector (int initial_allocation, int initial_size, 
            const T& init_val) throw()
    : data(NULL), length(initial_size), alloc(initial_allocation) 
    { do_realloc();
      for (int i = 0; i < length; i++) new(data+i) T(init_val); }

    /** Destructor. Nothing special. */
    ~Vector() throw() { if (data != NULL) free(data); }

    /** Return the size of the vector. */
    int size() const throw() { return length; }
    /** Resize the vector to size \c ns and allocate memory if necessary. */
    void resize(int ns) throw() { 
        int os = length;
        if (alloc < ns) { alloc = ns; do_realloc(); }
        length = ns;
        for (int i = os; i < ns; i++) new (data+i) T();
    }
    /** Resize the vector to size \c ns and allocate memory if necessary.
     *  Construct new elements by copying \c exemplar. */
    void resize(int ns, const T& exemplar) throw() { 
        int os = length;
        if (alloc < ns) { alloc = ns; do_realloc(); }
        for (int i = os; i < ns; i++) new (data+i) T(exemplar);
        length = ns;
    }
    /** Add the element \c element to the back of the Vector. */
    void push_back(T& element) throw() {
        realloc_if_necessary();
        new(data+length) T(element);
        length++;
    }
    /** Add the element \c element to the back of the Vector. */
    void push_back(const T& element) throw() {
        realloc_if_necessary();
        new(data+length) T(element);
        length++;
    }
    /** Make space for one additional element at the end of the
     *  vector, but do not initialize it. The user must call the
     *  appropriate constructor and then call commit().
     *
     *  @return A pointer to the newly allocated element. */
    T* allocate() throw() {
        realloc_if_necessary();
        return data+length;
    }
    /** Include the element that was previously allocated by
     *  allocate() into the vector. */
    void commit() throw() { length++; }

    /** Remove one element from the back of the Vector. */
    inline void pop_back() throw() { data[length-1].~T(); length--; }
    /** Remove all elements from the Vector. */
    inline void clear() throw() { while (this->length > 0) pop_back(); }
    /** Return a C array with the vector elements. */
    T* ptr() throw() { return data; }

    /** Access a single element at index \c i. */
    inline const T& operator[](int i) const throw() { return data[i]; }
    /** Access a single element at index \c i. */
    inline T& operator[](int i) throw() { return data[i]; }

    /** Access the first element of the vector. */
    inline const T& front() const throw() { return data[0]; }
    /** Access the last element of the vector. */
    inline const T& back() const throw() { return data[length-1]; }
    /** Access the first element of the vector. */
    inline T& front() throw() { return data[0]; }
    /** Access the last element of the vector. */
    inline T& back() throw() { return data[length-1]; }

    /** STL-like iterator class. */
    class const_iterator {
        protected:
        friend class Vector;
        const T *pos;
        public:
        const_iterator(const T* pos) : pos(pos) {}

        inline const T& operator*() const throw() { return *pos; }
        inline const T* operator->() const throw() { return pos; }
        inline void operator++(int) throw() { pos++; }
        inline void operator--(int) throw() { pos--; }
        inline bool operator==(const const_iterator &o) const throw()
            { return o.pos == pos; }
        inline bool operator!=(const const_iterator &o) const throw()
            { return o.pos != pos; }
        inline int operator-(const const_iterator &o) const throw()
            { return o.pos - pos; }
    };

    /** STL-like iterator class. */
    class iterator : public const_iterator {
        public:
        iterator(T* pos) : const_iterator(pos) {}

        inline T& operator*() throw() 
            { return *const_cast<T*>(this->pos); }
        inline T* operator->() throw() 
            { return const_cast<T*>(this->pos); }
    };

    /** Get an iterator to the first vector element. */
    inline const_iterator begin() const throw() 
        { return const_iterator(data); }
    /** Get an iterator to the first vector element. */
    inline iterator begin() throw() 
        { return iterator(data); }
    /** Get an iterator just past the last vector element. */
    inline const_iterator end() const throw() 
        { return const_iterator(data+length); }
    /** Get an iterator just past the last vector element. */
    inline iterator end() throw() 
        { return iterator(data+length); }

    /** Erase an vector element at the position indicated by the iterator. */
    iterator erase(iterator &i) throw() {
        T *first = const_cast<T*>(i.pos);
        first->~T();
        length--;
        memmove(first, first+1, ( (length) - (first-data) ) * sizeof(T));
        return i;
    }

};

}

#endif
