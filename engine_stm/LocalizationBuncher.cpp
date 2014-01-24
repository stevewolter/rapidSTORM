#include "debug.h"

#include "LocalizationBuncher.h"
#include <dStorm/input/Source.h>
#include <dStorm/output/Output.h>
#include <boost/variant/get.hpp>
#include <dStorm/localization/record.h>

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
    traits->in_sequence = true;

    return TraitsPtr( new TraitsPtr::element_type( *traits, "Buncher", "Localizations" ) );
}

template <class InputType>
Source<InputType>::Source( std::auto_ptr<Input> base )
    : base(base), input_left_over(false) {}

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
    while (true) {
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
            return;
        }
    }
}

template <class InputType>
void Source<InputType>::ReadImage(output::LocalizedImage* target) {
    while (true) {
        if (!input_left_over) {
            if (!base->GetNext(0, &input)) {
                return;
            }
        }

        frame_index input_image = GetImageNumber(input);
        assert(target->forImage <= input_image);
        if (in_sequence && target->forImage < input_image) {
            return;
        }

        if (target->forImage == input_image) {
            CollectEntireImage(target);
            return;
        } else {
            auto can = canned.emplace(input_image, input_image);
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
            *target = in_can->second;
            canned.erase(in_can);
            return true;
        }

        *target = output::LocalizedImage(current_image);
        ReadImage(target);
        return true;

        current_image += 1 * camera::frame;
    } catch (...) {
        return false;
    }
}

template class Source<localization::Record>;
template class Source<Localization>;

}
}
