#ifndef DSTORM_INPUT_ADAPTERSOURCE_H
#define DSTORM_INPUT_ADAPTERSOURCE_H

#include "Source.h"

namespace dStorm {
namespace input {

template <typename Type>
class AdapterSource
: public Source <Type>
{
    std::auto_ptr< Source<Type> > _base;
    virtual void modify_traits( Traits<Type>& ) {}
    simparm::Node& node() { return _base->getNode(); }
  protected:
    Source<Type>& base() { return *_base; }
    const Source<Type>& base() const { return *_base; }
    AdapterSource( std::auto_ptr< Source<Type> > b ) : _base(b) {}
  public:
    typename Source<Type>::iterator begin() { return _base->begin(); }
    typename Source<Type>::iterator end() { return _base->end(); }
    typename Source<Type>::TraitsPtr get_traits() { return _base->get_traits(); }
    void dispatch(typename Source<Type>::Messages m) { _base->dispatch(m); }
    typename Source<Type>::Capabilities capabilities() const 
        { return _base->capabilities(); }
    typename Source<Type>::TraitsPtr
        get_traits(typename Source<Type>::Wishes r) 
    { 
        typename Source<Type>::TraitsPtr p = _base->get_traits(r);
        modify_traits( *p );
        return p;
    }
};

}
}

#endif
