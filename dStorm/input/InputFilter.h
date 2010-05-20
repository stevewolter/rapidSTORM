#ifndef DSTORM_INPUT_ROIINPUTFILTER_H
#define DSTORM_INPUT_ROIINPUTFILTER_H

#include "Source.h"
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {
namespace input {

template <typename Type, typename FilterFunctor>
class InputFilter : public Source<Type>
{
    boost::shared_ptr< Source<Type> > upstream;
    typedef typename Source<Type>::iterator base_iterator;
    FilterFunctor filter;

  public:
    InputFilter( 
        std::auto_ptr< Source<Type> > upstream,
        const FilterFunctor& filter );
    InputFilter* clone() const 
        { return new InputFilter(*this); }

    base_iterator begin();
    base_iterator end();
    typename Source<Type>::TraitsPtr get_traits();
    void dispatch(BaseSource::Messages m);
};

}
}

#endif
