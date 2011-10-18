#ifndef DSTORM_INPUT_METAINFO_H
#define DSTORM_INPUT_METAINFO_H

#include "../Traits.h"
#include "../../output/Basename.h"
#include <set>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace input {
namespace chain {

struct MetaInfo {
    typedef boost::shared_ptr<MetaInfo> Ptr;
    typedef boost::shared_ptr<const MetaInfo> ConstPtr;

    virtual ~MetaInfo() {}
    virtual MetaInfo* clone() const { return new MetaInfo(*this); }

  private:
    boost::shared_ptr<BaseTraits> _traits;
  public:
    dStorm::output::Basename suggested_output_basename;
    std::set<std::string> forbidden_filenames;

    void set_traits( boost::shared_ptr<BaseTraits> t ) { _traits = (t); }
    void set_traits( BaseTraits* t ) { _traits.reset(t); }
    void set_traits( std::auto_ptr<BaseTraits> t ) { _traits.reset(t.release()); }
    template <typename Type>
    bool provides() const 
        { return dynamic_cast< Traits<Type>* >( _traits.get() ) != NULL; }
    BaseTraits& base_traits() { return *_traits; }
    const BaseTraits& base_traits() const { return *_traits; }
    template <typename Type>
    boost::shared_ptr< Traits<Type> > traits() 
        { return boost::dynamic_pointer_cast< Traits<Type>, BaseTraits >(_traits); }
    template <typename Type>
    boost::shared_ptr< const Traits<Type> > traits() const
        { return boost::dynamic_pointer_cast< const Traits<Type>, BaseTraits >(_traits); }
};

}
}
}

#endif
