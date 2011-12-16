#ifndef DSTORM_CONTEXT_H
#define DSTORM_CONTEXT_H

#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/quantity.hpp>
#include "../Traits.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace input {
namespace chain {

struct Context {
    typedef boost::shared_ptr<Context> Ptr;
    typedef boost::shared_ptr<const Context> ConstPtr;

    Context* clone() const { return new Context(*this); }

    boost::ptr_vector<BaseTraits> more_infos;

    template <typename Type> inline bool has_info_for() const;
    template <typename Type> inline Traits<Type>& get_info_for();
    template <typename Type> inline const Traits<Type>& get_info_for() const;
};

}
}
}

#endif
