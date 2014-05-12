#ifndef DSTORM_INPUT_ADAPTERSOURCE_H
#define DSTORM_INPUT_ADAPTERSOURCE_H

#include "input/Source.h"

namespace dStorm {
namespace input {

template <typename Type>
class AdapterSource
: public Source <Type>
{
    std::auto_ptr< Source<Type> > _base;
    virtual void modify_traits( Traits<Type>& ) {}
    virtual void attach_local_ui_( simparm::NodeHandle n ) = 0;
    void attach_ui_( simparm::NodeHandle n ) {
        attach_local_ui_(n);
        _base->attach_ui( n );
    }
  protected:
    Source<Type>& base() { return *_base; }
    const Source<Type>& base() const { return *_base; }
    AdapterSource( std::auto_ptr< Source<Type> > b ) : _base(b) {}
  public:
    void set_thread_count(int num_threads) OVERRIDE { _base->set_thread_count(num_threads); }
    bool GetNext(int thread, Type* target) OVERRIDE { return _base->GetNext(thread, target); }
    void dispatch(typename Source<Type>::Messages m) OVERRIDE { _base->dispatch(m); }
    typename Source<Type>::TraitsPtr get_traits() OVERRIDE
    { 
        typename Source<Type>::TraitsPtr p = _base->get_traits();
        modify_traits( *p );
        return p;
    }
};

}
}

#endif
