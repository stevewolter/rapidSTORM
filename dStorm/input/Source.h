/** \file dStorm/input/Source.h
 *  This file contains the definition for the Source class. */
#ifndef DSTORM_INPUT_SOURCE_H
#define DSTORM_INPUT_SOURCE_H

#include "fwd.h"
#include <stdexcept>
#include <memory>
#include <limits>
#include <bitset>
#include <simparm/Object.h>

#include "Traits.h"

#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm { 
namespace input { 
    /** A BaseSource class is an interface that supports
     *  delivery of a sequence of generated objects. 
     *
     *  Concrete source implementations should implement the
     *  templated Source classes.
     * */
    class BaseSource 
    {
      public:
        enum Capability { Repeatable, ConcurrentIterators, InputStandardDeviation };
        typedef std::bitset<3> Capabilities;
        typedef std::bitset<3> Wishes;
        enum Message {RepeatInput, WillNeverRepeatAgain};
        typedef std::bitset<2> Messages;

      private:
        virtual void attach_ui_( simparm::NodeHandle ) = 0;

      public:
        virtual ~BaseSource();

        void attach_ui( simparm::NodeHandle n ) { attach_ui_(n); }
        void dispatch(Message m) {
            dispatch(Messages().set(m));
        }
        virtual void set_thread_count(int num_threads) = 0;
        virtual void dispatch(Messages m) = 0;
        virtual Capabilities capabilities() const = 0;

        template <typename Type> bool can_provide() const;
        template <typename Type> Source<Type>& downcast() const;
        template <typename Type>
        inline static std::auto_ptr< Source<Type> >
            downcast( std::auto_ptr<BaseSource> );
    };

    /** A Source object of some given type. Provides default 
     *  implementations of get(), startPushing() and stopPushing().
     *
     *  Source objects derive from Traits objects for their respective
     *  types to publish the traits of the objects they deliver. 
     *  Therefore, any Traits definition should be visible when the
     *  Source object gets implemented. */
    template <class Type> class Source 
    : public BaseSource
    {
      public:
        typedef Type value_type;

        typedef input::Traits<Type> Traits;
        typedef boost::shared_ptr<Traits> TraitsPtr;
        typedef boost::shared_ptr<const Traits> ConstTraitsPtr;

        virtual bool GetNext(int thread, Type* output) = 0;
        virtual TraitsPtr get_traits(BaseSource::Wishes) = 0;
    };

    template <typename Type>
    bool BaseSource::can_provide() const 
        { return dynamic_cast< const Source<Type>* >( this ) != NULL; }

    template <typename Type> 
    Source<Type>&
    BaseSource::downcast() const
        { return dynamic_cast<Source<Type>&>(*this); }

    template <typename Type> 
    std::auto_ptr< Source<Type> >
    BaseSource::downcast(std::auto_ptr< BaseSource > p)
    {
        return std::auto_ptr< Source<Type> >
            ( &dynamic_cast<Source<Type>&>(*p.release()) );
    }

}
}

#endif
