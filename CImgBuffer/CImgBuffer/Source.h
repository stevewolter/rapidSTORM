/** \file Source.h
 *  This file contains the declaration for the Source class. */
#ifndef CImgBuffer_SOURCE_H
#define CImgBuffer_SOURCE_H

#include <stdexcept>
#include <memory>
#include <limits>
#include <simparm/Object.hh>
#include <CImgBuffer/Traits.h>
#include <CImgBuffer/Drain.h>

namespace CImgBuffer { 
    template <class T> class Drain;
    template <class T> class Source;

    /** A BaseSource class is an interface that supports
     *  delivery of a sequence of generated objects. 
     *
     *  BaseSource objects support two basic operation modes:
     *  Pulled and pushing. In the pulled mode, objects
     *  that are about to be fetched are announced via
     *  announce_fetch(), then fetched via fetch(). The
     *  Source object must keep all state it needs in
     *  permanent storage rather than in the stack. An
     *  example is the AndorSIF class.
     *
     *  In pushing mode, the BaseSource is given a Drain object
     *  that accepts its images. On calling startPushing(),
     *  it should start to concurrently insert acquired
     *  image into the Drain and continue to do so until
     *  it is empty or stopPushing() is called. It must
     *  make sure to fill all elements of the Drain; if
     *  necessary, with error elements.
     *
     *  Concrete source implementations should implement the
     *  templated Source classes.
     * */
    class BaseSource 
    : public virtual simparm::Node
    {
      public:
        static const int Pushing = 0x1, Pullable = 0x2, Managing = 0x4,
            Concurrent = 0x8;
        struct Flags {
            int value;
            Flags(int v) : value(v) {}
            Flags operator|(const Flags& o) const
                { return Flags(value | o.value); }
            bool operator&(const Flags& o) const
                { return (value & o.value) != 0; }
        };
      protected:
        bool _pushes, _canBePulled, _managed,
             _concurrent;
        unsigned int roi_start, roi_end;

        /** Number of objects that will be returned. */
        virtual int quantity() const = 0;

      public:
        BaseSource(Flags flags);
        virtual ~BaseSource();

        unsigned int number_of_objects() const;
        /** Set a region of interest in the objects. Only objects with
         *  source numbers from \c first_image to \c last_image will
         *  be returned. */
        void set_ROI(unsigned int first_image,
                     unsigned int last_image) 
            { roi_start = first_image; roi_end = last_image; }

        /** If true, startPushing may be called on this Source. */
        inline bool pushes() const { return _pushes; }
        /** If true, get may be called on this Source. */
        inline bool canBePulled() const { return _canBePulled; }

        /** Method to determine whether number  of objects can be asked via 
         *  number_of_objects().
         *  \return true Dimension can be asked.
         *  \return false Dimension can not (yet) be asked and will
         *          be given to the supplied Drain. */
        virtual bool pull_length() const { return true; }

        /** If true, the Source manages the returned objects on its
         *  own, and calling delete is not allowed. If false, the
         *  objects are dynamically allocated and must be deleted. */
        inline bool manages_returned_objects() const { return _managed; }
        /** If true, the startPushing method sends objects concurrently. 
         *  If false, it blocks the calling process until all objects
         *  were sent. */
        inline bool pushes_concurrently() const
            { return _concurrent; }
        
        template <typename Type> bool can_provide() const;
        template <typename Type> 
            std::auto_ptr< Source<Type> >
            downcast(std::auto_ptr< BaseSource > p) const;
    };

    /** A Source object of some given type. Provides default 
     *  implementations of get(), startPushing() and stopPushing(). */
    template <class Type> class Source 
    : public BaseSource, public Traits<Type>
    {
      protected:
        Drain<Type> *pushTarget;
        Source(const BaseSource::Flags&);
        virtual Type* fetch(int)
        { throw std::logic_error
                ("Tried to pull from an unpullable object source."); }

      public:
        typedef Type value_type;

        std::auto_ptr< Type > get(int index);
        virtual void startPushing(Drain<Type> *);
        virtual void stopPushing(Drain<Type> *) {}

        void allowPushing(Drain<Type>* to) { pushTarget = to; }
    };

    template <typename Type>
    bool BaseSource::can_provide() const 
        { return dynamic_cast< const Source<Type>* >( this ) != NULL; }

    template <typename Type> 
    std::auto_ptr< Source<Type> >
    BaseSource::downcast(std::auto_ptr< BaseSource > p) const
    {
        return std::auto_ptr< Source<Type> >
            ( dynamic_cast<Source<Type>*>(p.release()) );
    }

    template <typename Type>
    void Source<Type>::startPushing(Drain<Type> *drain)

    {
        for (unsigned int i = roi_start; i <= roi_end; i++) {
            Type *object = fetch(i);
            if ( object == NULL )
                return;
            else {
                Management m;
                m = drain->accept( i - roi_start, 1, object );
                if ( !manages_returned_objects() &&
                     m == Delete_objects )
                    delete object;
            }
        }
    }

    template <typename Type> 
    std::auto_ptr< Type >
    Source<Type>::get(int index) { 
        return std::auto_ptr<Type>(fetch(roi_start + index)); 
    }
};

#endif
