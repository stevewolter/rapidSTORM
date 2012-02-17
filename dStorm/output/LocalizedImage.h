#ifndef DSTORM_OUTPUT_LOCALIZED_IMAGE_H
#define DSTORM_OUTPUT_LOCALIZED_IMAGE_H

#include "../Localization.h"
#include "../units/frame_count.h"
#include "../engine/Image.h"
#include "../engine/CandidateTree_decl.h"

namespace dStorm {
namespace output {

struct LocalizedImage
{
    std::vector<Localization> results;
  public:
    /** Number of the image the localizations were found in. */
    frame_index forImage;
    /** If the SourceImage AdditionalData field was set,
        *  this pointer points to the image the localizations
        *  were computed in. */
    dStorm::engine::ImageStack source;
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
    LocalizedImage( const LocalizedImage& );
    ~LocalizedImage();
    LocalizedImage& operator=( const LocalizedImage& );

    typedef std::vector<Localization>::value_type value_type;
    typedef std::vector<Localization>::reference reference;
    typedef std::vector<Localization>::const_reference const_reference;
    typedef std::vector<Localization>::iterator iterator;
    typedef std::vector<Localization>::const_iterator const_iterator;
    size_t size() const { return results.size(); }
    Localization& operator[]( int i ) { return results[i]; }
    const Localization& operator[]( int i ) const { return results[i]; }
    std::vector<Localization>::const_iterator begin() const 
        { return results.begin(); }
    std::vector<Localization>::const_iterator end() const 
        { return results.end(); }
    std::vector<Localization>::iterator begin() 
        { return results.begin(); }
    std::vector<Localization>::iterator end()
        { return results.end(); }
    void push_back( const Localization& l );
    void clear();
    void resize( size_t size );
    const Localization& front() const { return results.front(); }
    Localization& front() { return results.front(); }
    Localization& back() { return results.back(); }
    const Localization& back() const { return results.back(); }
    iterator erase( iterator p );
    iterator erase( iterator f, iterator t );
    bool empty() const;

    frame_index frame_number() const { return forImage; }
    void set_frame_number(frame_index);
};

}
}

#endif
