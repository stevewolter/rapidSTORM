/** \file Source.h
 *  This file contains the definition for the Source class. */
#ifndef DSTORM_INPUT_SOURCE_H
#define DSTORM_INPUT_SOURCE_H

#include <stdexcept>
#include <memory>
#include <limits>
#include <bitset>
#include <simparm/Object.hh>

#include "Traits.h"
#include "Source_decl.h"

#include <any_iterator.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include "../output/Output_decl.h"
#ifndef BOOST_ENABLE_ASSERTS
#include "../output/Output.h"
#endif

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
        enum Capability { 
            TimeCritical, 
            Repeatable,
            MultipleConcurrentIterators };
        enum Message {RepeatInput, WillNeverRepeatAgain};
        typedef std::bitset<3> Capabilities;
        typedef Capabilities Flags;
        typedef std::bitset<2> Messages;
        const Flags flags;

      private:
        simparm::Node& node;

      public:
        BaseSource(simparm::Node& node, Flags flags);
        virtual ~BaseSource();

        simparm::Node& getNode() { return node; }
        operator simparm::Node&() { return node; }
        const simparm::Node& getNode() const { return node; }
        operator const simparm::Node&() const { return node; }

        void dispatch(Message m) {
            dispatch(Messages().set(m));
        }
        virtual void dispatch(Messages m) = 0;

        template <typename Type> bool can_provide() const;
        template <typename Type> Source<Type>& downcast() const;
        template <typename Type>
        inline static std::auto_ptr< Source<Type> >
            downcast( std::auto_ptr<BaseSource> );

        virtual boost::ptr_vector<output::Output>
            additional_outputs() { return boost::ptr_vector<output::Output>(); }
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
      protected:
        inline Source(simparm::Node& node, const BaseSource::Flags&);

      public:
        typedef Type value_type;

        typedef IteratorTypeErasure::any_iterator< Type, std::input_iterator_tag > iterator;
        typedef input::Traits<Type> Traits;
        typedef boost::shared_ptr<Traits> TraitsPtr;
        typedef boost::shared_ptr<const Traits> ConstTraitsPtr;

        virtual iterator begin() = 0;
        virtual iterator end() = 0;
        virtual TraitsPtr get_traits() = 0;
    };

    class Filter {
      public:
        virtual BaseSource& upstream() = 0;
        virtual boost::ptr_vector<output::Output>
            additional_outputs() { return upstream().additional_outputs(); }
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
