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

    MetaInfo();
    MetaInfo( const MetaInfo& );
    virtual ~MetaInfo();
    virtual MetaInfo* clone() const { return new MetaInfo(*this); }

  private:
    struct Signals;
    std::auto_ptr<Signals> _signals;
    boost::shared_ptr<BaseTraits> _traits;
  public:
    dStorm::output::Basename suggested_output_basename;
    std::set<std::string> forbidden_filenames;
    std::list< std::pair<std::string,std::string> > accepted_basenames;

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
    bool provides_nothing() const { return _traits.get() == NULL; }

    template <typename Type> Type& get_signal();
    template <typename Type> const Type& get_signal() const 
        { return const_cast<MetaInfo&>(*this).get_signal<Type>(); }
    void forward_connections( const MetaInfo& );
};

}
}
}

#endif
