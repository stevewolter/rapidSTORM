#ifndef LIBDATACPP_AUTOLIST_H
#define LIBDATACPP_AUTOLIST_H

#include <list>
#include <algorithm>
#include <functional>

namespace data_cpp {
template <typename Type>
class auto_list {
  private:
    std::list<Type*> base;
    typedef typename std::list<Type*>::iterator 
            base_iterator;
    typedef typename std::list<Type*>::const_iterator 
            base_const_iterator;

    void copy(const std::list<Type*>& from)
 
    {
        base.clear();
        std::transform( from.begin(), from.end(),
            back_inserter( base ), 
            std::mem_fun( &Type::clone ) );
    }

  public:
    auto_list() {}
    ~auto_list() { clear(); }

    auto_list(const auto_list<Type>& o) { copy(o.base); }
    auto_list& operator=(const auto_list<Type>& o) 
        { clear(); copy(o.base); }

    struct iterator : public base_iterator {
        iterator() {}
        iterator( const base_iterator& c ) : base_iterator(c) {}

        Type& operator*() const 
            { return **(base_iterator&)*this; }
        Type* operator->() const 
            { return *(base_iterator&)*this; }
    };
    struct const_iterator : public base_const_iterator {
        const_iterator() {}
        const_iterator( const base_const_iterator& c )
            : base_const_iterator(c) {}

        const Type& operator*() const 
            { return **(base_const_iterator&)*this; }
        const Type* operator->() const 
            { return *(base_const_iterator&)*this; }
    };

    iterator begin() { return base.begin(); }
    const_iterator begin() const { return base.begin(); }
    iterator end() { return base.end(); }
    const_iterator end() const { return base.end(); }

    const Type& front() const { return *base.front(); }
    const Type& back() const { return *base.back(); }
    Type& front() { return *base.front(); }
    Type& back() { return *base.back(); }

    size_t size() const { return base.size(); }

    void push_back(Type *p) { base.push_back(p); }
    void delete_now(Type *p) {
        for (base_iterator i = base.begin(); i != base.end(); i++)
            if ( *i == p ) {
                base.erase(i);
                delete p;
                break;
            }
    }
    void clear() {
        for (base_iterator i = base.begin(); i != base.end(); i++)
            delete *i;
        base.clear();
    }
};
}

#endif
