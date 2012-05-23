#ifndef BOOST_SMART_PTR_NOCOPY_PTR_HPP
#define BOOST_SMART_PTR_NOCOPY_PTR_HPP

#include <memory>

namespace dStorm {

template <typename Type>
class nocopy_ptr : public std::auto_ptr<Type> {
  public:
    nocopy_ptr() : std::auto_ptr<Type>() {}
    nocopy_ptr(const nocopy_ptr& o) : std::auto_ptr<Type>() {}
    nocopy_ptr(Type* t) : std::auto_ptr<Type>(t) {}
    nocopy_ptr(std::auto_ptr<Type> t) : std::auto_ptr<Type>(t) {}
    nocopy_ptr& operator=( const nocopy_ptr& o ) { return *this; }
    nocopy_ptr& operator=( nocopy_ptr& o ) { return *this; }
    template <typename T1>
    nocopy_ptr& operator=( T1 o ) { static_cast< std::auto_ptr<Type>& >(*this) = o; return *this; }
};

}

#endif
