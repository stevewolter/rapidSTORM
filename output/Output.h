#ifndef DSTORM_TRANSMISSION_H
#define DSTORM_TRANSMISSION_H

#include "output/Output_decl.h"

#include "engine/Image_decl.h"
#include "Localization.h"
#include "engine/Input_decl.h"
#include "engine/CandidateTree_decl.h"
#include <stdexcept>
#include "simparm/NodeHandle.h"
#include <iostream>
#include <set>
#include <bitset>

#include "output/Traits.h"
#include "output/Capabilities.h"
#include "units/frame_count.h"
#include "output/LocalizedImage.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace display { class Manager; }
namespace output {

class Engine;

/** Base interface for listeners to a rapidSTORM localization
    *  process. An Output will receive signals about the
    *  progress of the computation, get an initial notification
    *  about static run properties and receive many EngineResult
    *  structures which contain localizations to process. */
class Output {
public:
    struct Announcement;
    struct RunFinished {};
    typedef Capabilities AdditionalData;

    enum RunRequirement {
        MayNeedRestart
    };
    typedef std::bitset<1> RunRequirements;
    class RunAnnouncement {};

    /** An EngineResult structure is sent when localizations were
    *  found and are published to transmissions. */
    typedef LocalizedImage EngineResult;

protected:
    /** Method throws an exception when \c can_provide does not
        *  cover \c are_desired, and does nothing otherwise. */
    static void check_additional_data_with_provided
        (std::string name, AdditionalData can_provide, 
                            AdditionalData are_desired);
    static void insert_filename_with_check(
        std::string file, std::set<std::string>& present_filenames );

    virtual void store_results_(bool) {}
    virtual void prepare_destruction_() {}
    virtual void run_finished_(const RunFinished&) {}
    virtual void attach_ui_( simparm::NodeHandle ) {}

public:
    virtual ~Output() {}

    virtual void check_for_duplicate_filenames
        (std::set<std::string>& present_filenames) {}

    void attach_ui( simparm::NodeHandle );

    /** This method is called before the rapidSTORM engine is run. It's
        *  parameters are the width and the height of a source image and the
        *  number of source images.
        *  @return A bitfield indicating which additional data (besides the
        *          source image number and the found localizations) should
        *          be transmitted with receiveLocalizations(). */
    virtual AdditionalData announceStormSize(const Announcement&) = 0;
    virtual RunRequirements announce_run(const RunAnnouncement&) 
        { return RunRequirements(); }
    virtual void receiveLocalizations(const EngineResult&) = 0;
    void store_results( bool job_successful ) { store_results_(job_successful); }
    void prepare_destruction() { prepare_destruction_(); }
    void run_finished( const RunFinished& i ) { run_finished_(i); }
};

class Output::Announcement
: public input::Traits<LocalizedImage>
{
  public:
    Announcement( 
        const input::Traits<LocalizedImage>& traits );
};

}
}

#endif
