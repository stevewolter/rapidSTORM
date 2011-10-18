#ifndef DSTORM_CONTEXT_IMPL_H
#define DSTORM_CONTEXT_IMPL_H

#include "Context.h"

namespace dStorm {
namespace input {
namespace chain {

template <typename Type>
bool Context::has_info_for() const {
    for ( boost::ptr_vector<BaseTraits>::const_iterator
          i = this->more_infos.begin(); i != this->more_infos.end(); ++i )
        if ( dynamic_cast<const Traits<Type>*>(&*i) != NULL )   
            return true;
    return false;
}

template <typename Type> 
Traits<Type>& Context::get_info_for() {
    for ( boost::ptr_vector<BaseTraits>::iterator
          i = this->more_infos.begin(); i != this->more_infos.end(); ++i )
    {
        Traits<Type>* rv = dynamic_cast<Traits<Type>*>(&*i);
        if ( rv ) return *rv;
    }
    assert(false);
    throw std::logic_error("Tried to access non-existent info in context");
}

template <typename Type> 
const Traits<Type>& Context::get_info_for() const {
    for ( boost::ptr_vector<BaseTraits>::const_iterator
          i = this->more_infos.begin(); i != this->more_infos.end(); ++i )
    {
        const Traits<Type>* rv = dynamic_cast<const Traits<Type>*>(&*i);
        if ( rv ) return *rv;
    }
    assert(false);
    throw std::logic_error("Tried to access non-existent info in context");
}


}
}
}

#endif
