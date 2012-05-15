#ifndef SIMPARM_DATACHOICEENTRY_ASSIGNMENT_HH
#define SIMPARM_DATACHOICEENTRY_ASSIGNMENT_HH

#include "ChoiceEntry.hh"

namespace simparm {

template <typename DataType>
DataChoiceEntry<DataType>& DataChoiceEntry<DataType>::operator=(const DataType& choice)
{
    for (typename NodeChoiceEntry< DataChoice<DataType> >::
            iterator i = this->beginChoices();
                                    i != this->endChoices(); i++)
        if ( (*i)() == choice ) {
            this->value = &*i;
            break;
        }
    return *this;
}

}

#endif
