#ifndef DSTORM_INPUT_MASKFILTER_H
#define DSTORM_INPUT_MASKFILTER_H

#include "debug.h"
#include <boost/optional/optional.hpp>
#include <simparm/FileEntry.hh>
#include <boost/units/io.hpp>
#include <dStorm/input/AdapterSource.h>
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
: public AdapterSource<Ty>
{
    std::vector<bool> mask;

    class _iterator;

  public:
    Mask(std::auto_ptr< Source<Ty> > backend, const MaskConfig& config);
    
    typedef typename Source<Ty>::iterator iterator;
    typedef typename Source<Ty>::TraitsPtr TraitsPtr;
    typedef Ty value_type;

    iterator begin();
    iterator end();
};

}
}

#endif

