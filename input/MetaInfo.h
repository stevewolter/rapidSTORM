#ifndef DSTORM_INPUT_METAINFO_H
#define DSTORM_INPUT_METAINFO_H

#include "input/fwd.h"
#include "input/Traits.h"
#include "output/Basename.h"
#include <set>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace input {

struct MetaInfo {
    typedef boost::shared_ptr<MetaInfo> Ptr;
    typedef boost::shared_ptr<const MetaInfo> ConstPtr;

    MetaInfo();
    MetaInfo( const MetaInfo& );
    virtual ~MetaInfo();
    virtual MetaInfo* clone() const { return new MetaInfo(*this); }

  private:
    struct Signals;
    std::auto_ptr<Signals> _signals;
    boost::shared_ptr<const BaseTraits> _traits;
  public:
    dStorm::output::Basename suggested_output_basename;
    std::set<std::string> forbidden_filenames;
    std::list< std::pair<std::string,std::string> > accepted_basenames;

    void set_traits( boost::shared_ptr<const BaseTraits> t ) { _traits = (t); }
    void set_traits( const BaseTraits* t ) { _traits.reset(t); }
    template <typename Type>
    bool provides() const 
        { return dynamic_cast< const Traits<Type>* >( _traits.get() ) != NULL; }
    const BaseTraits& base_traits() const { return *_traits; }
    template <typename Type>
    boost::shared_ptr< const Traits<Type> > traits() const
        { return boost::dynamic_pointer_cast< const Traits<Type>, const BaseTraits >(_traits); }
    bool provides_nothing() const { return _traits.get() == NULL; }

    template <typename Type> Type& get_signal();
    template <typename Type> const Type& get_signal() const 
        { return const_cast<MetaInfo&>(*this).get_signal<Type>(); }
    void forward_connections( const MetaInfo& );
};

}
}

#endif
