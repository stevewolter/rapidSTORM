#ifndef DSTORM_INPUT_BUFFER_IMPL_ITERATOR_H
#define DSTORM_INPUT_BUFFER_IMPL_ITERATOR_H

#include <cassert>
#include "Buffer.h"
#include <boost/shared_ptr.hpp>
#include <boost/iterator/iterator_facade.hpp>

namespace dStorm {
namespace input {

template<typename Type, bool RunConcurrently>
class Buffer<Type,RunConcurrently>::iterator
: public boost::iterator_facade<iterator,Type,std::forward_iterator_tag>
{
  public:
    iterator();
    iterator(Buffer& buffer);

  private:
    class referenced;
    mutable boost::shared_ptr<referenced> content;
    friend class boost::iterator_core_access;

    Type& dereference() const { return **content; }
    bool equal(const iterator& o) const 
        { return content.get() == o.content.get(); }
    void increment();

    bool isValid();
};

template<typename Type, bool RunConcurrently>
struct Buffer<Type,RunConcurrently>::iterator::referenced
{
    Buffer& b;
    typename Slots::iterator c;

  public:
    referenced(Buffer& buffer) 
        : b(buffer), c(b.get_free_slot()) {}
    boost::shared_ptr<referenced> advance() 
        { return boost::shared_ptr<referenced>(new referenced(b)); }
    ~referenced();

    Type& operator*() { return *c; }
    const Type& operator*() const { return *c; }
    bool check() { return c != b.buffer.end(); }
};

template<typename Type, bool RunConcurrently>
Buffer<Type,RunConcurrently>::iterator::iterator() 
{
}

template<typename Type, bool RunConcurrently>
Buffer<Type,RunConcurrently>::iterator::iterator(Buffer<Type,RunConcurrently>& buffer)
{
    content.reset( new referenced( buffer ) );
    if ( ! content->check() ) content.reset();
}

template<typename Type, bool RunConcurrently>
Buffer<Type,RunConcurrently>::iterator::referenced::~referenced() {
    b.discard( c );
}

template<typename Type, bool RunConcurrently>
void Buffer<Type,RunConcurrently>::iterator::increment() 
{
    if ( content.get() != NULL ) content = content->advance();
    if ( ! content->check() ) content.reset();
}

}
}

#endif
