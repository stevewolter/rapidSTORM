#ifndef ANDOR_CAMERA_STATE_MACHINE_IMPL_H
#define ANDOR_CAMERA_STATE_MACHINE_IMPL_H

#include "StateMachine.h"

namespace AndorCamera {

template <typename ReachedStateProducer>
std::auto_ptr<States::Token>
StateMachine::StandardListener<ReachedStateProducer>::raise_state (States::State to)
{
    std::auto_ptr<States::Token> rv;
    switch ( to ) {
        case States::Disconnected: 
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Disconnected>(p) );
            break;
        case States::Connecting: 
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Connecting>(p) );
            break;
        case States::Connected:
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Connected>(p) );
            break;
        case States::Readying:
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Readying>(p) );
            break;
        case States::Ready:
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Ready>(p) );
            break;
        case States::Acquiring:
            rv.reset( new typename ReachedStateProducer::
                    template Token<States::Acquiring>(p) );
            break;
        default:
            throw std::logic_error(
                "Unknown state encountered in raise_state");
    }
    return rv;
}

}

#define MK_EMPTY_RW(classname) \
    template <int State> \
    struct classname ::Token : public States::Token { \
        classname &parent; \
        Token( classname &); \
        ~Token(); \
    }; \
    \
    template <int State> \
    classname::Token<State>::Token(classname& p) : parent(p) {} \
    template <int State> \
    classname::Token<State>::~Token() {}



#endif
