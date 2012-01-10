#include "debug.h"

#include "LocalizationBuncher.h"
#include <dStorm/input/Source.h>
#include <dStorm/output/Output.h>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <dStorm/localization/record.h>

using namespace dStorm::output;

namespace dStorm {
namespace engine_stm {

enum VisitResult { KeepComing, IAmFinished, FinishedAndReject };

class Visitor 
: public boost::static_visitor<VisitResult>
{
    std::auto_ptr<output::LocalizedImage> can;
    boost::optional<frame_index> my_image;

  public:
    ~Visitor() {}
    VisitResult operator()( const dStorm::Localization& l ) 
    {
        DEBUG( "Got localization " << l.frame_number() );
        if ( my_image.is_initialized() ) {
            if ( l.frame_number() != *my_image )
                return FinishedAndReject;
        } else {
            my_image = l.frame_number();
            can.reset( new output::LocalizedImage() );
            can->forImage = *my_image;
        }
        can->push_back( l );
        return KeepComing;
    }

    VisitResult operator()( const dStorm::localization::EmptyLine& i ) 
    {
        DEBUG( "Got empty line " << i.number );
        default_to( i.number );
        return IAmFinished;
    }

    template <typename Type> VisitResult add(Type& argument);
    frame_index for_frame() { return *my_image; }
    std::auto_ptr<output::LocalizedImage> get_result() { 
        assert( can.get() );
        return can; 
    }
    void default_to( frame_index i ) { 
        if ( ! my_image.is_initialized() ) {
            my_image = i;
            can.reset( new output::LocalizedImage(i) );
        }
    }
};

template <typename Type>
VisitResult 
Visitor::add(Type& argument)
{
    return boost::apply_visitor( *this, argument );
}

template <>
VisitResult 
Visitor::add<Localization>(Localization& argument)
{
    return (*this)( argument );
}

template <typename Input>
LocalizationBuncher<Input>::LocalizationBuncher( Source<Input>* master, frame_index first_image )
: master(master), outputImage( first_image - 1 * camera::frame )
{
    if ( master ) { increment(); }
}

template <typename Input>
void LocalizationBuncher<Input>::increment() {
    output.reset();
    outputImage += 1 * camera::frame;
    if ( master->is_finished( outputImage ) ) {
        DEBUG("Not serving after " << outputImage);
        master = NULL;
    } else {
        output = master->read( outputImage );
        DEBUG("Serving " << output->forImage);
    }
}

template <typename Input>
bool LocalizationBuncher<Input>::equal(const LocalizationBuncher<Input>& o) const
{
    return master == o.master && ( master == NULL || outputImage == o.outputImage );
}

template <typename Input>
std::auto_ptr<output::LocalizedImage> 
Source<Input>::read( frame_index outputImage )
{
    DEBUG("Seeking " << outputImage);
    while ( canned.find(outputImage) == canned.end() )
    {
        Visitor v;
        for ( ; current != base_end; ++current )  {
            VisitResult r = v.add(*current);
            if ( r == FinishedAndReject )
                break;
            else if ( r == IAmFinished ) {
                ++current;
                break;
            }
        }
        v.default_to( outputImage );
        DEBUG("Visitor declares to be finished with " << v.for_frame() << ", want " << outputImage);

        if ( in_sequence ) {
            for ( frame_index f = outputImage; f < v.for_frame(); 
                    f += 1 * camera::frame ) 
            {
                DEBUG("Putting " << f << " into the can");
                canned.insert( f, new output::LocalizedImage(f) );
            }
        }

        DEBUG("Putting " << v.for_frame() << " into the can");
        canned.insert( v.for_frame(), v.get_result() );
    }
    DEBUG("Removing " << outputImage << " from the can");
    assert( canned.find(outputImage) != canned.end() );
    std::auto_ptr<output::LocalizedImage> rv( 
        canned.release(canned.find(outputImage)).release() );
    assert( rv.get() );
    return rv;
}

template <typename Input>
LocalizationBuncher<Input>::LocalizationBuncher
    (const LocalizationBuncher& o)
: master( const_cast< Source<Input>* >(o.master) ),
  output(o.output),
  outputImage(o.outputImage)
{
}

template <typename Input>
LocalizationBuncher<Input>::~LocalizationBuncher() {
}

template <typename InputType>
typename Source<InputType>::TraitsPtr
Source<InputType>::get_traits( Wishes w )
{
    w.reset( input::BaseSource::ConcurrentIterators );
    input::Source<Localization>::TraitsPtr traits  = base->get_traits( w );
    traits::ImageNumber::RangeType& r = traits->image_number().range();
    this->in_sequence = traits->in_sequence;
    DEBUG("Localizations are " << ((this->in_sequence) ? "" : "not") << " in sequence");

    current = base->begin();
    base_end = base->end();

    if ( ! r.first.is_initialized() )
        throw std::runtime_error("First image index in STM file must be known");
    first_image = *r.first;
    last_image = r.second;
    traits->in_sequence = true;

    return TraitsPtr( new TraitsPtr::element_type( *traits, "Buncher", "Localizations" ) );
}

template <class InputType>
Source<InputType>::Source( std::auto_ptr<Input> base )
: base(base) {}

template <class InputType>
Source<InputType>::~Source()
{
    canned.clear();
    current = base_end = InputIterator();
}

template <class InputType>
void Source<InputType>::dispatch(Messages m)
{
    if ( m.test( RepeatInput ) ) {
        m.reset( RepeatInput );
        current = base->begin();
        base_end = base->end();
    }
    m.reset( WillNeverRepeatAgain );
    base->dispatch(m);
}

template <class InputType>
bool Source<InputType>::is_finished( frame_index at_image ) const { 
    if ( last_image.is_initialized() )
        return *last_image < at_image;
    else
        return current == base_end && canned.empty();
}

template class Source<localization::Record>;
template class Source<Localization>;

}
}
