#ifndef DSTORM_INPUT_MASKFILTER_H
#define DSTORM_INPUT_MASKFILTER_H

#include "debug.h"
#include <simparm/optional.hh>
#include <simparm/FileEntry.hh>
#include <boost/units/io.hpp>
#include <dStorm/input/Source.h>
#include <dStorm/input/Config.h>
#include <vector>

namespace dStorm {
namespace input {

class MaskConfig 
: public simparm::Object
{
  public:
    MaskConfig();

    simparm::FileEntry mask_image;
    void registerNamedEntries() { push_back( mask_image ); }
};

template <typename Ty>
class Mask
: public Source<Ty>, public Filter
{
    std::auto_ptr< Source<Ty> > s;
    std::vector<bool> mask;

    class _iterator;

  public:
    Mask(std::auto_ptr< Source<Ty> > backend, const MaskConfig& config);
    
    BaseSource& upstream() { return *s; }

    typedef typename Source<Ty>::iterator iterator;
    typedef typename Source<Ty>::TraitsPtr TraitsPtr;
    typedef Ty value_type;

    void dispatch(BaseSource::Messages m) { s->dispatch(m); }
    iterator begin();
    iterator end();
    TraitsPtr get_traits() { return s->get_traits(); }
};

}
}

#endif

