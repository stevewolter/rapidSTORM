#include "debug.h"

#include <boost/lexical_cast.hpp>
#include <boost/variant/get.hpp>

#include "engine_stm/LocalizationBuncher.h"
#include "input/Source.h"
#include "localization/record.h"
#include "output/Output.h"

using namespace dStorm::output;

namespace dStorm {
namespace engine_stm {

Source::TraitsPtr Source::get_traits() {
    input::Source<Localization>::TraitsPtr traits  = base->get_traits();
    auto& r = traits->image_number().range();
    this->in_sequence = traits->in_sequence;
    DEBUG("Localizations are " << ((this->in_sequence) ? "" : "not") << " in sequence");

    if ( ! r.first.is_initialized() )
        throw std::runtime_error("First image index in STM file must be known");
    first_image = *r.first;
    last_image = r.second;
    traits->in_sequence = true;

    TraitsPtr result( new TraitsPtr::element_type( *traits, "Buncher", "Localizations" ) );
    result->group_field = input::GroupFieldSemantic::ImageNumber;
    return result;
}

Source::Source( std::unique_ptr<Input> base )
    : base(std::move(base)), input_left_over(false), input_exhausted(false) {}

Source::~Source()
{
    canned.clear();
}

void Source::dispatch(Messages m)
{
    if ( m.test( RepeatInput ) ) {
        current_image = first_image;
    }
    m.reset( WillNeverRepeatAgain );
    base->dispatch(m);
}

void Source::set_thread_count(int threads) {
    if (threads != 1) {
        throw std::logic_error("Can only read localizations single-threaded");
    }

    base->set_thread_count(threads);
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

void Source::CollectEntireImage(output::LocalizedImage* target) {
    assert(input_left_over);
    while (true) {
        if (boost::get<const localization::EmptyLine>(&input)) {
            input_left_over = false;
            return;
        } else if (const Localization* localization = boost::get<const Localization>(&input)) {
            if (localization->frame_number().value() == target->group) {
                target->push_back(*localization);
            } else {
                input_left_over = true;
                return;
            }
        } else {
            throw std::logic_error("Unhandled localization record type");
        }

        if (!base->GetNext(0, &input)) {
            DEBUG("Exhausted input while reading image " << target->group);
            input_exhausted = true;
            input_left_over = false;
            return;
        }
    }
}

void Source::ReadImage(output::LocalizedImage* target) {
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
        if (in_sequence && target->group < input_image.value()) {
            return;
        }
        if (target->group > input_image.value()) {
            throw std::runtime_error("Duplicate image " +
                    boost::lexical_cast<std::string>(input_image) +
                    " in input");
        }

        if (target->group == input_image.value()) {
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

bool Source::GetNext(int thread, output::LocalizedImage* target) {
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

}
}
