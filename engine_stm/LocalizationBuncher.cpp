#include "debug.h"

#include "engine_stm/LocalizationBuncher.h"
#include "input/Source.h"
#include "output/Output.h"
#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>
#include "localization/record.h"

using namespace dStorm::output;

namespace dStorm {
namespace engine_stm {

enum VisitResult { KeepComing, IAmFinished, FinishedAndReject };

template <typename InputType>
typename Source<InputType>::TraitsPtr
Source<InputType>::get_traits( Wishes w )
{
    w.reset( input::BaseSource::ConcurrentIterators );
    input::Source<Localization>::TraitsPtr traits  = base->get_traits( w );
    auto& r = traits->image_number().range();
    this->in_sequence = traits->in_sequence;
    DEBUG("Localizations are " << ((this->in_sequence) ? "" : "not") << " in sequence");

    if ( ! r.first.is_initialized() )
        throw std::runtime_error("First image index in STM file must be known");
    first_image = *r.first;
    last_image = r.second;
    traits->in_sequence = true;

    return TraitsPtr( new TraitsPtr::element_type( *traits, "Buncher", "Localizations" ) );
}

template <class InputType>
Source<InputType>::Source( std::auto_ptr<Input> base )
    : base(base), input_left_over(false), input_exhausted(false) {}

template <class InputType>
Source<InputType>::~Source()
{
    canned.clear();
}

template <class InputType>
void Source<InputType>::dispatch(Messages m)
{
    if ( m.test( RepeatInput ) ) {
        current_image = first_image;
    }
    m.reset( WillNeverRepeatAgain );
    base->dispatch(m);
}

template <class InputType>
void Source<InputType>::set_thread_count(int threads) {
    if (threads != 1) {
        throw std::logic_error("Can only read localizations single-threaded");
    }

    base->set_thread_count(threads);
}

frame_index GetImageNumber(const Localization& input) {
    return input.frame_number();
}

frame_index GetImageNumber(const dStorm::localization::Record& input) {
    if (const localization::EmptyLine* line = boost::get<localization::EmptyLine>(&input)) {
        return line->number;
    } else if (const Localization* localization = boost::get<dStorm::Localization>(&input)) {
        return localization->frame_number();
    } else {
        throw std::logic_error("Unhandled localization record type");
    }
}

VisitResult AddInputToImage(output::LocalizedImage* target, const Localization& input) {
    if (input.frame_number() == target->forImage) {
        target->push_back(input);
        return KeepComing;
    } else {
        return FinishedAndReject;
    }
}

VisitResult AddInputToImage(output::LocalizedImage* target, const localization::Record& input) {
    if (boost::get<const localization::EmptyLine>(&input)) {
        return IAmFinished;
    } else if (const Localization* localization = boost::get<const Localization>(&input)) {
        return AddInputToImage(target, *localization);
    } else {
        throw std::logic_error("Unhandled localization record type");
    }
}

template <typename InputType>
void Source<InputType>::CollectEntireImage(output::LocalizedImage* target) {
    assert(input_left_over);
    while (true) {
        DEBUG("Trying to add localization from " << GetImageNumber(input)
              << " to image " << target->forImage);
        switch (AddInputToImage(target, input)) {
            case KeepComing:
                break;
            case IAmFinished:
                input_left_over = false;
                return;
            case FinishedAndReject:
                input_left_over = true;
                return;
        }

        if (!base->GetNext(0, &input)) {
            DEBUG("Exhausted input while reading image " << target->forImage);
            input_exhausted = true;
            input_left_over = false;
            return;
        }
    }
}

template <class InputType>
void Source<InputType>::ReadImage(output::LocalizedImage* target) {
    while (true) {
        if (!input_left_over) {
            if (!base->GetNext(0, &input)) {
                input_exhausted = true;
                return;
            } else {
                input_left_over = true;
            }
        }

        frame_index input_image = GetImageNumber(input);
        if (in_sequence && target->forImage < input_image) {
            return;
        }
        if (target->forImage > input_image) {
            throw std::runtime_error("Duplicate image " +
                    boost::lexical_cast<std::string>(input_image) +
                    " in input");
        }

        if (target->forImage == input_image) {
            DEBUG("Immediately delivering image " << input_image);
            CollectEntireImage(target);
            return;
        } else {
            DEBUG("Putting image " << input_image << " into can");
            auto can = canned.insert(std::make_pair(input_image, output::LocalizedImage(input_image)));
            CollectEntireImage(&can.first->second);
        }
    }
}

template <class InputType>
bool Source<InputType>::GetNext(int thread, output::LocalizedImage* target) {
    assert(thread == 0);

    try {
        auto in_can = canned.find(current_image);
        if (in_can != canned.end()) {
            DEBUG("Delivering image " << current_image << " from can");
            *target = in_can->second;
            canned.erase(in_can);
            current_image += 1 * camera::frame;
            return true;
        }

        if (canned.empty() && input_exhausted &&
            (!last_image || current_image > *last_image)) {
            DEBUG("Can empty and input exhausted, not delivering an image");
            return false;
        }

        *target = output::LocalizedImage(current_image);
        if (!input_exhausted) {
            ReadImage(target);
        }
        current_image += 1 * camera::frame;
        return true;
    } catch (...) {
        return false;
    }
}

template class Source<localization::Record>;
template class Source<Localization>;

}
}
