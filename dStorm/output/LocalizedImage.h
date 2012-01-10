#ifndef DSTORM_OUTPUT_LOCALIZED_IMAGE_H
#define DSTORM_OUTPUT_LOCALIZED_IMAGE_H

#include "../Localization.h"
#include "../units/frame_count.h"
#include "../engine/Image.h"
#include "../engine/CandidateTree_decl.h"

namespace dStorm {
namespace output {

struct LocalizedImage
: public std::vector<Localization>
{
    /** Number of the image the localizations were found in. */
    frame_index forImage;
    /** If the SourceImage AdditionalData field was set,
        *  this pointer points to the image the localizations
        *  were computed in. */
    dStorm::engine::Image source;
    /** If the SmoothedImage AdditionalData field was set,
        *  this pointer points to the image where candidates
        *  were found in. */
    const dStorm::engine::SmoothedImage *smoothed;
    /** If the CandidateTree AdditionalData field was set,
        *  this pointer points to the candidate merging tree. */
    const dStorm::engine::CandidateTree<dStorm::engine::SmoothedPixel>
        *candidates;

    LocalizedImage();
    LocalizedImage(frame_index);

    frame_index frame_number() const { return forImage; }
    void set_frame_number(frame_index);
};

}
}

#endif
