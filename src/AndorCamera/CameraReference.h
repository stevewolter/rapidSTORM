/** \file CameraReference.h
 *  This file contains the AndorCamera::CameraReference class.
 **/
#ifndef ANDORCAMERA_CAMERAREFERENCE_H
#define ANDORCAMERA_CAMERAREFERENCE_H

#include <exception>
#include <string>

/** \namespace AndorCamera
 *
 *  The AndorCamera namespace provides a safe C++ interface to the
 *  Andor SDK functions. 
 *
 *  The typical use would consist of:
 *  \li Obtaining a reference to the Camera singleton with System
 *  \li Adjusting the camera parameters in Camera
 *  \li Allocating an Acquisition object and starting it
 *  \li Retrieving images from the Acquisition object
 **/
namespace AndorCamera {
    class Camera; 
    
    /** A reference-counting class controlling access to Camera.
     *  This is the only supported method of accessing the Camera class.
     **/
    class CameraReference {
        private:
            int num;
            Camera *getCamObject() const;

        public:
            /** Get a new reference for the camera with number 
             *  camNumber. */
            CameraReference(int camNumber);

            /** Copy an existing reference to Camera.
             *  This method is exception-safe since it never
             *  causes construction of Camera. */
            CameraReference(const CameraReference &);

            /** Destruct this reference and, if possible, Camera. */
            ~CameraReference();

            /** Get a reference to Camera.
             *  This reference is valid as long as this object lives. */
            Camera& operator*() { return *getCamObject(); }
            /** Get a pointer to Camera.
             *  This pointer is valid as long as this object lives.
             *  Do not delete the pointer; deallocation is the 
             *  responsibility of this class. */
            Camera* operator->() { return getCamObject(); }
            /** Get a const reference to Camera.
             *  This reference is valid as long as this object lives. */
            const Camera& operator*() const 
                { return *getCamObject(); }
            /** Get a const pointer to Camera.
             *  This pointer is valid as long as this object lives. */
            const Camera* operator->() const 
                { return getCamObject(); }
    };

}

#endif
