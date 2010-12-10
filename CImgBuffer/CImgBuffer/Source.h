/** \file Source.h
 *  This file contains the declaration for the Source class. */
#ifndef CImgBuffer_SOURCE_H
#define CImgBuffer_SOURCE_H

#include <stdexcept>
#include <memory>
#include <ConfigObject.hh>

namespace CImgBuffer { 
    template <class T> class Drain;

    /** A Source class is an interface that supports
     *  delivery of a sequence of generated objects. 
     *
     *  Source objects support two basic operation modes:
     *  Pulled and pushing. In the pulled mode, objects
     *  that are about to be fetched are announced via
     *  announce_fetch(), then fetched via fetch(). The
     *  Source object must keep all state it needs in
     *  permanent storage rather than in the stack. An
     *  example is the AndorSIF class.
     *
     *  In pushing mode, the Source is given a Drain object
     *  that accepts its images. On calling startPushing(),
     *  it should start to concurrently insert acquired
     *  image into the Drain and continue to do so until
     *  it is empty or stopPushing() is called. It must
     *  make sure to fill all elements of the Drain; if
     *  necessary, with NULL elements.
     * */
    template <class T> class Source {
      private:
        bool _pushes, _canBePulled;
      protected:
        Drain<T> *pushTarget;
        Source(bool pushes = false, bool canBePulled = true) throw()
            : _pushes(pushes), _canBePulled(canBePulled), pushTarget(NULL)
            {}

      public:
        virtual ~Source() {}

        virtual std::auto_ptr< T > 
            fetch(int /* index */) throw(std::exception) 
            { throw std::logic_error
                ("Tried to pull from an unpullable object source."); }
        virtual int dimx() throw() = 0;
        virtual int dimy() throw() = 0;
        virtual int dimz() throw() { return 1; }
        virtual int dimv() throw() { return 1; }
        virtual int image_quantity() throw() = 0;

        inline bool pushes() const { return _pushes; }
        inline bool canBePulled() const { return _canBePulled; }

        virtual void startPushing(Drain<T> *) throw(std::exception) 
            { throw std::logic_error("Tried to make a source push "
                     "Objects that must be pulled."); }
        virtual void stopPushing(Drain<T> *) throw(std::exception) {}

        virtual simparm::ConfigObject& getConfig() throw() = 0;
   };
};

#endif
