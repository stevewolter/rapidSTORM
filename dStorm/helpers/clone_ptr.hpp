#ifndef BOOST_SMART_PTR_CLONE_PTR_HPP
#define BOOST_SMART_PTR_CLONE_PTR_HPP

#include <memory>

namespace boost {

template <typename Type>
class clone_ptr : public std::auto_ptr<Type> {
  public:
    clone_ptr() : std::auto_ptr<Type>() {}
    clone_ptr(const clone_ptr& o) 
        : std::auto_ptr<Type>( o.get() ? new_clone(*o) : NULL ) {}
    clone_ptr(Type* t) : std::auto_ptr<Type>(t) {}
    clone_ptr(std::auto_ptr<Type> t) : std::auto_ptr<Type>(t) {}
};

}

#endif
