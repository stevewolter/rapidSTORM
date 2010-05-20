#ifndef DSTORM_INPUT_INPUTFILTER_IMPL_H
#define DSTORM_INPUT_INPUTFILTER_IMPL_H

#include "InputFilter.h"
#include "Source_impl.h"
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace dStorm {
namespace input {

template <typename Ty, typename Filter>
InputFilter<Ty,Filter>::InputFilter( 
        std::auto_ptr< Source<Ty> > upstream,
        const Filter& filter 
)
: Source<Ty>( upstream->getNode(), upstream->flags ),
  upstream(upstream),
  filter(filter)
{
}

template <typename Ty, typename Filter>
typename Source<Ty>::iterator InputFilter<Ty,Filter>::begin() 
{
    return base_iterator(
        boost::filter_iterator<Filter,base_iterator>
        ( filter, this->upstream->begin(), this->upstream->end() ) );
}

template <typename Ty, typename Filter>
typename Source<Ty>::iterator InputFilter<Ty,Filter>::end() 
{
    return base_iterator(
        boost::filter_iterator<Filter,base_iterator>
        ( filter, this->upstream->end(), this->upstream->end() ) );
}

template <typename Ty, typename Filter>
void InputFilter<Ty,Filter>::dispatch(BaseSource::Messages m) 
{
    upstream->dispatch(m);
}

template <typename Ty, typename Filter>
typename Source<Ty>::TraitsPtr
InputFilter<Ty,Filter>::get_traits() 
{
    return filter(upstream->get_traits());
}

}
}

#endif
