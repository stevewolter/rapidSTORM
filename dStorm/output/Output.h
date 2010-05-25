#ifndef DSTORM_TRANSMISSION_H
#define DSTORM_TRANSMISSION_H

#include <dStorm/Localization.h>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Input_decl.h>
#include <dStorm/engine/CandidateTree_decl.h>
#include <stdexcept>
#include <simparm/Set.hh>
#include <iostream>
#include <set>
#include <bitset>
#include <dStorm/helpers/thread.h>

#include "Traits.h"
#include "Capabilities.h"
#include <dStorm/units/frame_count.h>

namespace dStorm {
namespace output {
    class ResultRepeater;

    /** Base interface for listeners to a rapidSTORM localization
     *  process. An Output will receive signals about the
     *  progress of the computation, get an initial notification
     *  about static run properties and receive many EngineResult
     *  structures which contain localizations to process. */
    class Output {
        simparm::Node& node;
      public:
        class Announcement;
        typedef Capabilities AdditionalData;

        enum RunRequirement {
            MayNeedRestart
        };
        typedef std::bitset<1> RunRequirements;
        class RunAnnouncement {};

        class EngineResult;
        enum Result { KeepRunning, RemoveThisOutput, RestartEngine,
                        StopEngine };
        enum ProgressSignal { 
            /** This signal is sent just before the rapidSTORM engine is
            *  starting or restarting. It is sent once per computation
            *  thread. */
            Engine_run_is_starting,
            /** This signal is sent when any piston terminates the engine
            *  run. It is called in the context of the first thread that
            *  terminates the run.
            *  Outputs must make sure that control flows of all
            *  threads leaves the transmission upon receival of this
            *  signal. */
            Engine_run_is_aborted,
            /** This signal is sent after all pistons aborted work and
             *  the engine prepares to relaunch. */
            Engine_is_restarted,
            /** This signal is sent after all pistons aborted work and
             *  the engine decides to abort the run. */
            Engine_run_failed,
            /** This signal is sent after all pistons finished work. */
            Engine_run_succeeded,
            /** This signal indicates that the job is completed 
             *  successfully, files should be closed and saves committed.*/
            Job_finished_successfully,
            /** This signal is sent when the engine has completed operations
             *  and is prepared to destruct transmissions. Any transmission
             *  that cannot be destructed yet should block the reception
             *  of this signal. */
            Prepare_destruction 
        };


      public:
        friend std::ostream &
            operator<<(std::ostream &o, Result r);
        friend std::ostream &
            operator<<(std::ostream &o, ProgressSignal r);

      private:
        Output(const Output&);
        Output& operator=(const Output&);
      protected:
        Output(simparm::Node& node) : node(node) {}
        Output(simparm::Node& node, const Output&) : node(node) {}
        /** Method throws an exception when \c can_provide does not
         *  cover \c are_desired, and does nothing otherwise. */
        static void check_additional_data_with_provided
            (std::string name, AdditionalData can_provide, 
                               AdditionalData are_desired);
        static void insert_filename_with_check(
            std::string file, std::set<std::string>& present_filenames );

      public:
        virtual ~Output() {}
        virtual Output* clone() const = 0;

        virtual void check_for_duplicate_filenames
            (std::set<std::string>& present_filenames) {}

        simparm::Node& getNode() { return node; }
        operator simparm::Node&() { return node; }
        const simparm::Node& getNode() const { return node; }
        operator const simparm::Node&() const { return node; }

        /** This method is called before the rapidSTORM engine is run. It's
         *  parameters are the width and the height of a source image and the
         *  number of source images.
         *  @return A bitfield indicating which additional data (besides the
         *          source image number and the found localizations) should
         *          be transmitted with receiveLocalizations(). */
        virtual AdditionalData announceStormSize(const Announcement&) = 0;
        virtual RunRequirements announce_run(const RunAnnouncement&) 
            { return RunRequirements(); }
        virtual void propagate_signal(ProgressSignal) = 0;
        virtual Result receiveLocalizations(const EngineResult&)
            = 0;
    };

    /** This structure summarizes the static information
     *  sent at run start. */
    struct Output::Announcement {
        /** Traits of the source images, including dimensions and
         *  resolution. */
        const Traits traits;

        /** If the data source knows which carburettor supplies the
          * images, this pointer is set to it, and NULL otherwise. */
        dStorm::engine::Input* carburettor;
        /** This pointer is set to the last structure able to repeat
          * the delivered results, if any. NULL otherwise. */
        ResultRepeater *result_repeater;

        Announcement(
            const Traits& traits,
            dStorm::engine::Input* carburettor = NULL,
            ResultRepeater *repeater = NULL);
    };

    /** An EngineResult structure is sent when localizations were
     *  found and are published to transmissions. */
    struct Output::EngineResult {
        /** Number of the image the localizations were found in. */
        frame_index forImage;
        /** Pointer to array of localizations. May be NULL if
         *  number is 0, must be given otherwise.*/
        const Localization *first;
        /** Number of localizations in array. */
        int number;
        /** If the SourceImage AdditionalData field was set,
         *  this pointer points to the image the localizations
         *  were computed in. */
        const dStorm::engine::Image *source;
        /** If the SmoothedImage AdditionalData field was set,
         *  this pointer points to the image where candidates
         *  were found in. */
        const dStorm::engine::SmoothedImage *smoothed;
        /** If the CandidateTree AdditionalData field was set,
         *  this pointer points to the candidate merging tree. */
        const dStorm::engine::CandidateTree<dStorm::engine::SmoothedPixel>
            *candidates;
    };

    class OutputObject : public simparm::Object, public Output {
        OutputObject& operator=(const OutputObject&);
      public:
        OutputObject(const std::string& name, const std::string& desc)
            : simparm::Object(name, desc), 
              Output( static_cast<simparm::Node&>(*this) ) {}
        OutputObject(const OutputObject& o)
            : simparm::Object(o), Output(*this, o) {}
        ~OutputObject() {}
        OutputObject* clone() const = 0;
    };

}
}

#endif
