#ifndef DSTORM_INPUT_BUFFER_IMPL_ITERATOR_H
#define DSTORM_INPUT_BUFFER_IMPL_ITERATOR_H

#include <cassert>
#include "Buffer.h"
#include <boost/shared_ptr.hpp>

namespace dStorm {
namespace input {

template <typename Type>
class Buffer<Type>::iterator {
  public:
    typedef std::forward_iterator_tag iterator_category;
    typedef Type value_type;
    typedef ptrdiff_t difference_type;
    typedef Type* pointer;
    typedef Type& reference;

    iterator();

    Type& operator*() { return **content; }
    const Type& operator*() const { return **content; }
    Type* operator->() { return &**content; }
    const Type* operator->() const { return &**content; }

    iterator& operator++();
    iterator operator++(int);

    iterator& attach(Buffer& buffer);

    bool operator==(const iterator& o) const { return o.content == content; }
    bool operator!=(const iterator& o) const { return o.content != content; }

  private:
    class referenced;
    boost::shared_ptr<referenced> content;

    bool isValid();
};

template <typename Type>
struct Buffer<Type>::iterator::referenced
{
    Buffer& b;
    typename Slots::iterator c;

  public:
    referenced(Buffer& buffer) 
        : b(buffer), c(b.get_free_slot()) {}
    boost::shared_ptr<referenced> advance() {
        boost::shared_ptr<referenced> r(new referenced(b)); 
        if ( r->c == b.current.end() )
            r.reset();
        return r;
    }
    ~referenced();

    Type& operator*() { return *c; }
    const Type& operator*() const { return *c; }
};

template <typename Type>
typename Buffer<Type>::iterator&
Buffer<Type>::iterator::attach(Buffer<Type>& buffer)
{
    content.reset( new referenced( buffer ) );
}

template <typename Type>
Buffer<Type>::iterator::referenced::~referenced() {
    b.discard( c );
}

template <typename Type>
typename Buffer<Type>::iterator&
Buffer<Type>::iterator::operator++() 
{
    if ( content.get() != NULL )
        content = content->advance();
    return *this;
}

template <typename Type>
typename Buffer<Type>::iterator
Buffer<Type>::iterator::operator++(int) 
{
    iterator i = *this;
    ++ (*this);
    return i;
}

}
}

#endif
