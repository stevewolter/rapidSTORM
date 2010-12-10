/** \file ControlStateListener.h
 *  
 *  This file contains the ControlStateListener class declaration */
#ifndef ANDORCAMERA_CONTROLSTATELISTENER_H
#define ANDORCAMERA_CONTROLSTATELISTENER_H

#include <AndorCamera/ControlReference.h>

namespace AndorCamera {
    /** Base class that notifies derived objects of changes in
      * the Control state. */
    class ControlStateListener {
        private:
            friend class Control;
            /** This function is called by Control every time its
             *  \c state changes. */
            static void notify(State newState) throw(std::exception);

        protected:
            /** The constructor registers the listener at the Control. */
            ControlStateListener() throw();
            /** The destructor unregisters the listener from the Control.*/
            virtual ~ControlStateListener() throw();

            /** This function is called every time the state of Control
             *  changes. It is called for every state transition; thus,
             *  if the Control class goes from Inquired to Acquiring,
             *  newState is called five times - with Initialized, Cooling,
             *  Cooled, Configured and Acquiring as arguments, in this
             *  order.
             *
             *  \param newState The state the control just went to.
             **/
            virtual void controlStateChanged(State newState) 
                throw(std::exception) = 0;
    };
}

#endif
