#ifndef DSTORM_SMALLVECTOR_H
#define DSTORM_SMALLVECTOR_H

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <cassert>

#include <iterator>

#include <data-c++/Traits.h>

namespace data_cpp {

template <typename Type, bool nontrivial>
struct _Destructor {
    static void destruct(Type &o) { o.Type::~Type(); }
};

template <typename Type>
struct _Destructor<Type, false> {
    static void destruct(Type &) {}
};

template <typename Type>
struct Destructor {
    static void destruct(Type &o) { 
        _Destructor<Type, Traits<Type>::NontrivialDestructor>::
                destruct(o);
    }
};

template <typename Type, bool MemMoveNecessitatesCopy>
struct Memmover
{
    static inline void move( Type* to, const Type* from,
                             size_t length )
    {
        for (size_t i = 0; i < length; i++) {
            new (to+i) Type( from[i] );
            Destructor<Type>::destruct( from[i] );
        }
    }
    static inline void move( Type* to, Type* from,
                             size_t length )
    {
        for (size_t i = 0; i < length; i++) {
            new (to+i) Type( from[i] );
            Destructor<Type>::destruct( from[i] );
        }
    }
    static inline void overlap_move
        ( Type* to, const Type* from, size_t length )
        { move( to, from, length ); }
    static inline void copy( Type* to, const Type* from,
                             size_t length )
    {
        for (size_t i = 0; i < length; i++) {
            new (to+i) Type( from[i] );
        }
    }
};

template <typename Type>
struct Memmover<Type, false>
{
    static inline void move( Type* to, const Type* from,
                             size_t length )
    {
        memcpy( to, from, sizeof(Type) * length );
    }
    static inline void overlap_move( Type* to, const Type* from,
                             size_t length )
    {
        memmove( to, from, sizeof(Type) * length );
    }
    static inline void copy( Type* to, const Type* from,
                             size_t length )
    {
        memcpy( to, from, sizeof(Type) * length );
    }
};

/** The Vector class allows storage for a single data vector.
 *  It allocates sufficient storage to hold all data elements
 *  and provides direct access to elements or a C vector con-
 *  taining them.
 *
 *  The Vector class uses malloc() for storage allocation. If
 *  the malloc() implementation is thread-safe, concurrent
 *  accesses to different vectors are safe as well.
 */
template <typename Type>
class Vector {
  protected:
    Type *data;
    int length, alloc;

    typedef Memmover<Type, Traits<Type>::MemMoveNecessitatesCopy>
        MemMover;
    
    inline void do_realloc() {
        Type *new_data = (Type*)malloc(sizeof(Type) * alloc);
        if (new_data == NULL) 
            throw std::bad_alloc();
        if ( data != NULL ) {
            MemMover::move( new_data, data, length );
            free(data);
        }
        data = new_data;
    }
    inline void realloc_if_necessary(int number = 1) 
    {
        if (length + number > alloc) {
            alloc *= 2;
            do_realloc();
        }
    }

    /** Clone the exemplar \c init_val into the
     *  vector elements [from,from+length-1]. */
    inline void clone_entries(
        const Type& init_val, int from, int length) 
    {
        if ( length == 0 ) return;
        Type *data = this->data + from;
        if ( Traits<Type>::NontrivialCopy )
            for (int i = 0; i < length; i++) 
                new(data+i) Type(init_val); 
        else {
            new(data) Type(init_val); 
            int i = 1;
            while ( 2*i <= length ) {
                memcpy( data + i, data, sizeof(Type)*i );
                i *= 2;
            }
            int j = i / 2;
            while ( j != 0 ) {
                if ( (length & j) != 0 ) {
                    memcpy( data + i, data, sizeof(Type)*j );
                    i += j;
                }
                j /= 2;
            }
            assert( i == length );
        }
    }

  public:
    /** Construct an empty vector with a capacity of 10. */
    Vector () 
    : data(NULL), length(0), alloc(10) 
    { do_realloc(); }
    /** Construct an empty vector with a size and capacity of \c initialSize. */
    Vector (int initialSize)
    : data(NULL), length(initialSize), alloc(initialSize) 
    { do_realloc();
      for (int i = 0; i < length; i++) new(data+i) Type(); }
    /** Construct an empty vector with a size and capacity of \c initialSize
     *  and copy its values from init_val. */
    Vector (int initial_size, const Type& init_val)
    : data(NULL), length(initial_size), alloc(initial_size) 
    { do_realloc(); clone_entries(init_val, 0, initial_size); 
    }
    /** Construct an empty vector with a size of \c initial_size and capacity
     *  of \c initial_allocation and copy its values from init_val. */
    Vector (int initial_allocation, int initial_size, 
            const Type& init_val) 
    : data(NULL), length(initial_size), alloc(initial_allocation) 
    { do_realloc(); clone_entries(init_val, 0, initial_size); }

    /** There is no copy constructor for a Vector for efficiency reasons.*/
    Vector (const Vector<Type>& copy_from) 
    : data(NULL), length(0), alloc(0)
    { (*this) = copy_from; }
    /** No assignment as well */
    Vector<Type>& operator=(const Vector<Type>& from) 
    {
        if ( this == &from ) return *this;
        if ( alloc == 0 || alloc < from.length ) {
            alloc = std::max(10, from.length);
            do_realloc();
        }
        MemMover::copy( data, from.data, length );
        return *this;
    }

    /** Destructor. Nothing special. */
    ~Vector() { 
        clear();
        assert( data != NULL );
        free(data); 
    }

    /** Return the size of the vector. */
    int size() const { return length; }
    /** Resize the vector to size \c ns and allocate memory if necessary. */
    void resize(int ns) { 
        int os = length;
        if (alloc < ns) { alloc = ns; do_realloc(); }
        length = ns;
        for (int i = os; i < ns; i++) new (data+i) Type();
    }
    /** Resize the vector to size \c ns and allocate memory if necessary.
     *  Construct new elements by copying \c exemplar. */
    void resize(int ns, const Type& exemplar) { 
        int os = length;
        if (alloc < ns) { alloc = ns; do_realloc(); }
        clone_entries( exemplar, os, ns-os );
        length = ns;
    }
    /** Add the element \c element to the back of the Vector. */
    void push_back(Type& element) {
        realloc_if_necessary();
        new(data+length) Type(element);
        length++;
    }
    /** Add the element \c element to the back of the Vector. */
    void push_back(const Type& element) {
        realloc_if_necessary();
        new(data+length) Type(element);
        length++;
    }
    /** Add all elements of the vector \c v to the back of the Vector. */
    void push_back(const Vector<Type>& v) {
        int new_length = length + v.length;
        if ( new_length > alloc ) 
            { alloc = std::max<int>(alloc*2,new_length); do_realloc(); }
        MemMover::copy( data + length, v.data, v.length );
        length = new_length;
    }
    /** Make space for one additional element at the end of the
     *  vector, but do not initialize it. The user must call the
     *  appropriate constructor and then call commit().
     *
     *  @return A pointer to the newly allocated element. */
    Type* allocate(int number = 1) {
        realloc_if_necessary(number);
        return data+length;
    }
    /** Include the element that was previously allocated by
     *  allocate() into the vector. */
    void commit(int number = 1) { length += number; }

    /** Remove one element from the back of the Vector. */
    inline void pop_back() { 
        Destructor<Type>::destruct( data[length-1] );
        length--; 
    }
    /** Remove all elements from the Vector. */
    inline void clear() { 
        if (Traits<Type>::NontrivialDestructor)
            while (this->length > 0) pop_back(); 
        else
            length = 0;
    }
    /** Return a C array with the vector elements. */
    Type* ptr() { return data; }

    /** Access a single element at index \c i. */
    inline const Type& operator[](int i) const { return data[i]; }
    /** Access a single element at index \c i. */
    inline Type& operator[](int i) { return data[i]; }

    /** Access the first element of the vector. */
    inline const Type& front() const { return data[0]; }
    /** Access the last element of the vector. */
    inline const Type& back() const { return data[length-1]; }
    /** Access the first element of the vector. */
    inline Type& front() { return data[0]; }
    /** Access the last element of the vector. */
    inline Type& back() { return data[length-1]; }

  private:
    /** STL-like iterator class. */
    template <typename ReturnType, typename ConvType = ReturnType>
    class _iterator 
        : public std::iterator<std::random_access_iterator_tag, 
                               ReturnType> 
    {
      protected:
        friend class Vector;
        ReturnType *pos;
      public:
        _iterator(ReturnType* pos) : pos(pos) {}
        _iterator(const _iterator<ConvType>& o) 
            : pos(o.pos) {}

        inline ReturnType& operator*() const { return *pos; }
        inline ReturnType* operator->() const { return pos; }
        inline _iterator operator++(int) 
            { return _iterator(pos++); }
        inline _iterator operator--(int)
            { return _iterator(pos--); }
        inline _iterator& operator++() { pos++; return *this; }
        inline _iterator& operator--() { pos--; return *this; }
        inline _iterator& operator+=(int i) 
            { pos += i; return *this; }
        inline _iterator& operator-=(int i) 
            { pos -= i; return *this; }
        inline _iterator operator+(int i) const 
            { return _iterator(pos+i); }
        inline _iterator operator-(int i) const 
            { return _iterator(pos-i); }
        inline bool operator==(const _iterator &o) const
            { return o.pos == pos; }
        inline bool operator!=(const _iterator &o) const
            { return o.pos != pos; }
        inline bool operator<(const _iterator &o) const
            { return pos < o.pos; }
        inline bool operator>(const _iterator &o) const
            { return pos > o.pos; }
        inline bool operator>=(const _iterator &o) const
            { return pos >= o.pos; }
        inline bool operator<=(const _iterator &o) const
            { return pos <= o.pos; }

        friend inline _iterator operator+(int i, const _iterator& j)
            { return j - i; }
        friend inline _iterator operator-(int i, const _iterator& j)
            { return j - i; }

        inline ReturnType& operator[](int i) const
            { return pos[i]; }
    };
  public:
    typedef _iterator<Type, const Type> iterator;
    typedef _iterator<const Type> const_iterator;

    /** Get an iterator to the first vector element. */
    inline const_iterator begin() const 
        { return const_iterator(data); }
    /** Get an iterator to the first vector element. */
    inline iterator begin() 
        { return iterator(data); }
    /** Get an iterator just past the last vector element. */
    inline const_iterator end() const 
        { return const_iterator(data+length); }
    /** Get an iterator just past the last vector element. */
    inline iterator end() 
        { return iterator(data+length); }

    /** Erase an vector element at the position indicated by the iterator. */
    iterator erase(iterator &i) {
        Type *first = const_cast<Type*>(i.pos);
        Destructor<Type>::destruct(*first);
        length--;
        MemMover::memmove( first, first+1, length - (first-data) );
        return i;
    }

    inline void swap( Vector<Type>& with ) {
        std::swap( data, with.data );
        std::swap( length, with.length );
        std::swap( alloc, with.alloc );
    }
};

}

namespace std {
    template <typename Type>
    inline void swap( data_cpp::Vector<Type>& a, data_cpp::Vector<Type>& b)
        { a.swap( b ); }
}

#endif
