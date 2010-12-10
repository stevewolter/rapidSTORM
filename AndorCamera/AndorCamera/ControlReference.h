/** \file ControlReference.h
 *  This file contains the AndorCamera::ControlReference class.
 **/
#ifndef ANDORCAMERA_CONTROLREFERENCE_H
#define ANDORCAMERA_CONTROLREFERENCE_H

#include <exception>
#include <string>

/** \namespace AndorCamera
 *
 *  The AndorCamera namespace provides a safe C++ interface to the
 *  Andor SDK functions. 
 *
 *  The typical use would consist of:
 *  \li Obtaining a reference to the Control singleton with
 *      ControlReference
 *  \li Adjusting the camera parameters in Control
 *  \li Allocating an Acquisition object and starting it
 *  \li Retrieving images from the Acquisition object
 **/
namespace AndorCamera {
    class Control; 
    
    /** A reference-counting class controlling access to Control.
     *  This is the only supported method of accessing the Control class.
     **/
    class ControlReference {
        private:
            /** Store the reference to AndorCamera::Control
             *  to avoid multiple calls to Control::getControl . */
            Control *control;

        public:
            /** Get a new reference for the Control singleton.
             *  If the Control singleton needs to be constructed,
             *  the exception may be thrown. */
            ControlReference() throw(std::exception);
            /** Copy an existing reference to Control.
             *  This method is exception-safe since it never
             *  causes construction of Control. */
            ControlReference(const ControlReference &) throw();

            /** Destruct this reference and, if possible, Control.
             *  If an error occurs in destructing, it is printed
             *  to stderr rather than thrown. If you would prefer
             *  to handle the error, use release() prior to destruction. */
            ~ControlReference() throw();

            /** Release the reference to Control. Invalidates the
             *  operator*() and operator->() methods and might throw
             *  an exception if the Control object is deleted. 
             *  Use this method rather than ~ControlReference() if you
             *  want to handle exceptions. */
            void release() throw(std::exception);

            /** Get a reference to Control.
             *  This reference is valid as long as this object lives. */
            Control& operator*() throw() { return *control; }
            /** Get a pointer to Control.
             *  This pointer is valid as long as this object lives.
             *  Do not delete the pointer; deallocation is the 
             *  responsibility of this class. */
            Control* operator->() throw() { return control; }
            /** Get a const reference to Control.
             *  This reference is valid as long as this object lives. */
            const Control& operator*() const throw() { return *control; }
            /** Get a const pointer to Control.
             *  This pointer is valid as long as this object lives. */
            const Control* operator->() const throw() { return control; }
    };

    /** The possible states an Andor camera might be in. */
    enum State { 
        /** Starting state, no connection to camera. 
         *  In this state only it is possible to switch between cameras. */
        Inquired = 0,    
        /** Connected to camera, but cooler is still off. */
        Initialized = 1, 
        /** Camera is cooling, but did not necessarily reach the 
         *  required temperature yet. */
        Cooling = 2,     
        /** Camera did reach target temperature */
        Cooled = 3, 
        /** Applied and locked all configuration parameters. */
        Configured = 4, 
        /** Camera is acquiring */
        Acquiring = 5 
    };

}

#endif
