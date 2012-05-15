#ifndef SIMPARM_LINK_ITERATOR_HH
#define SIMPARM_LINK_ITERATOR_HH

#include "Link.hh"

namespace simparm {

template <class From, class To>
template <class ValueType, bool up> 
class Link<From,To>::_const_iterator
: public std::iterator< std::bidirectional_iterator_tag, const ValueType >
{
    typedef typename Link<From,To>::List::const_iterator BaseType;
    typedef typename Link<From,To>::List::iterator NonConstBaseType;

    typedef _const_iterator<ValueType,up> OwnType;
    typedef _iterator<ValueType,up> NonConstType;

    BaseType base;
  public:
    _const_iterator() {}
    _const_iterator( const BaseType& base ) : base(base) {}
    _const_iterator( const NonConstBaseType& base ) : base(base) {}
    _const_iterator( const NonConstType& c_other ) 
        : base(c_other.get_base()) {}

    const ValueType& operator*() const 
        { return (up) ? (*base)->up_end() : (*base)->down_end(); }
    const ValueType* operator->() const 
        { return &(*(*this)); }

    bool operator==(const OwnType& o)
        { return o.base == base; }
    bool operator!=(const OwnType& o)
        { return o.base != base; }

    OwnType& operator++() { ++base; return *this; }
    OwnType operator++(int) { return OwnType(base++); }
    OwnType& operator--() { --base; return *this; }
    OwnType operator--(int) { return OwnType(base--); }

    const BaseType& get_base() const { return base; }
    const Link<From,To>& get_link() const { return **base; }
};

template <class From, class To>
template <class ValueType, bool up> 
class Link<From,To>::_iterator
: public std::iterator< std::bidirectional_iterator_tag, ValueType >
{
    typedef typename Link<From,To>::List::iterator BaseType;
    typedef _iterator<ValueType,up> OwnType;

    BaseType base;
  public:
    _iterator() {}
    _iterator( const BaseType& base ) : base(base) {}

    ValueType& operator*() const 
        { return (up) ? (*base)->up_end() : (*base)->down_end(); }
    ValueType* operator->() const 
        { return &(*(*this)); }

    bool operator==(const _iterator<ValueType,up>& o)
        { return o.base == base; }
    bool operator!=(const _iterator<ValueType,up>& o)
        { return o.base != base; }

    OwnType& operator++() { ++base; return *this; }
    const OwnType operator++(int) { return OwnType(base++); }
    OwnType& operator--() { --base; return *this; }
    const OwnType operator--(int) { return OwnType(base--); }

    const BaseType& get_base() const { return base; }
    void break_link() const { delete *base; }
    Link<From,To>& get_link() const { return **base; }
};

}

#endif
