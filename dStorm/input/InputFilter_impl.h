#ifndef DSTORM_INPUT_INPUTFILTER_IMPL_H
#define DSTORM_INPUT_INPUTFILTER_IMPL_H

#include "InputFilter.h"
#include "Source_impl.h"
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/filter_iterator.hpp>

namespace dStorm {
namespace input {

template <typename Ty, typename FilterFunctor>
InputFilter<Ty,FilterFunctor>::InputFilter( 
        std::auto_ptr< Source<Ty> > upstream,
        const FilterFunctor& filter 
)
: Source<Ty>( upstream->getNode(), upstream->flags ),
  _upstream(upstream),
  filter(filter)
{
}

template <typename Ty, typename FilterFunctor>
typename Source<Ty>::iterator InputFilter<Ty,FilterFunctor>::begin() 
{
    return base_iterator(
        boost::filter_iterator<FilterFunctor,base_iterator>
        ( filter, this->_upstream->begin(), this->_upstream->end() ) );
}

template <typename Ty, typename FilterFunctor>
typename Source<Ty>::iterator InputFilter<Ty,FilterFunctor>::end() 
{
    return base_iterator(
        boost::filter_iterator<FilterFunctor,base_iterator>
        ( filter, this->_upstream->end(), this->_upstream->end() ) );
}

template <typename Ty, typename FilterFunctor>
void InputFilter<Ty,FilterFunctor>::dispatch(BaseSource::Messages m) 
{
    _upstream->dispatch(m);
}

template <typename Ty, typename FilterFunctor>
typename Source<Ty>::TraitsPtr
InputFilter<Ty,FilterFunctor>::get_traits() 
{
    return filter(_upstream->get_traits());
}

}
}

#endif
