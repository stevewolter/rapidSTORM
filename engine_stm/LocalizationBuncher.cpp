#include "debug.h"

#include "LocalizationBuncher.h"
#include <dStorm/input/Source_impl.h>
#include <dStorm/output/Output.h>
#include <dStorm/output/Trace.h>
#include <boost/variant/apply_visitor.hpp>

using namespace dStorm::output;

namespace dStorm {
namespace engine_stm {

class LocalizationBuncher::Can
{
    std::list< output::Trace > traces;

    void deep_copy(const Localization& from, 
                        output::Trace& to);
  public:
    Can() { traces.push_back( output::Trace() ); }
    void push_back( const Localization& l );
    void write( output::LocalizedImage& );
};

class LocalizationBuncher::Visitor 
: public boost::static_visitor<VisitResult>
{
    std::auto_ptr<Can> can;
    simparm::optional<frame_index> my_image;

  public:
    ~Visitor() { assert( can.get() == NULL ); }
    VisitResult operator()( const dStorm::Localization& l ) 
    {
        if ( my_image.is_set() ) {
            if ( l.frame_number() != *my_image )
                return IAmFinished;
            else
                can->push_back( l );
        } else {
            my_image = l.frame_number();
            can.reset( new Can() );
        }
        return KeepComing;
    }

    VisitResult operator()( const dStorm::LocalizationFile::EmptyLine& i ) 
    {
        if ( ! my_image.is_set() ) {
            my_image = i.number;
            can.reset( new Can() );
        }
        return IAmFinished;
    }

    frame_index for_frame() { assert(my_image.is_set()); return *my_image; }
    std::auto_ptr<Can> get_result() { return can; }
};

LocalizationBuncher::LocalizationBuncher(
    const Config& config,
    Base base,
    Base base_end,
    frame_index image_number)
: base(base), base_end(base_end), outputImage(image_number)
{
    search_output_image();
}

void LocalizationBuncher::increment() {
    output.reset();
    outputImage += 1 * cs_units::camera::frame;
    Canned::iterator canned_output = canned.find(outputImage);
    if ( canned_output == canned.end() )
        search_output_image();
    else
        output.reset( canned.release(canned_output).release() );
}

bool LocalizationBuncher::equal(const LocalizationBuncher& o) const
{
    return outputImage == o.outputImage;
}

void LocalizationBuncher::search_output_image() {
    while ( base != base_end ) {
        Visitor v;
        for ( ; base != base_end; ++base ) 
            if ( boost::apply_visitor(v, *base) == IAmFinished )
                break;
        if ( v.for_frame() == outputImage ) {
            output = v.get_result();
            break;
        } else
            canned.insert( v.for_frame(), v.get_result() );
    }
    if ( output.get() == NULL )
        /* End of file reached. Insert empty can. */
        output.reset( new Can() );

    output->write( result );
}

LocalizationBuncher::~LocalizationBuncher() {
}

Source::TraitsPtr Source::get_traits()
{
    input::Source<Localization>::TraitsPtr traits  = base->get_traits();

    if ( ! traits->last_frame.is_set() )
        throw std::runtime_error("Total number of frames in STM file must be known");
    firstImage = traits->first_frame;
    lastImage = *traits->last_frame + 1 * cs_units::camera::frame;
    firstImage = std::min(firstImage, lastImage);

    return TraitsPtr( new TraitsPtr::element_type( *traits ) );

#if 0
    if ( data.test( output::Capabilities::SourceImage ) ) {
        std::stringstream message;
        message << "One of your output modules needs access to the raw "
                << "images of the acquisition. These are not present in "
                << "a localizations file. Either remove the output or "
                << "choose a non-text input file";
        throw std::runtime_error(message.str());
    } else if (
        data.test( output::Capabilities::SmoothedImage) ||
        data.test( output::Capabilities::CandidateTree) ||
        data.test( output::Capabilities::InputBuffer) ) 
    {
        std::stringstream ss;
        ss << 
            "A selected data processing function "
            "requires data about the input data ("
            << data << ") that are not "
            "present in a localization file.";
        throw std::runtime_error(ss.str());
    }
#endif
}

void LocalizationBuncher::Can::push_back( const Localization &loc )
{
    deep_copy( loc, traces.front() );
}

void LocalizationBuncher::Can::deep_copy( 
    const Localization &loc, output::Trace& to )
{
    Localization *target = to.allocate(1);
    *target = loc;
    if ( loc.has_source_trace() ) {
        traces.push_back( output::Trace() );
        Trace& my_trace = traces.back();
        const Trace& t = loc.get_source_trace();
        for ( Trace::const_iterator i = t.begin(); i != t.end(); i++)
            deep_copy( *i, my_trace );
        target->set_source_trace( my_trace );
    }
    to.commit(1);
}

void LocalizationBuncher::Can::write( output::LocalizedImage& er ) 
{
    er.first = traces.front().ptr();
    er.number = traces.front().size();
}

}
}
