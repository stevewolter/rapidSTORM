#ifndef DSTORM_TRANSMISSION_H
#define DSTORM_TRANSMISSION_H

#include <dStorm/Localization.h>
#include <dStorm/Image.h>
#include <stdexcept>
#include <cc++/thread.h>
#include <simparm/Set.hh>
#include <iostream>

namespace CImgBuffer {
    template <typename PixelType> class Image;
}

namespace dStorm {
    template <typename PixelType> class CandidateTree;
    class Input;
    class ResultRepeater;

    /** Base interface for listeners to a rapidSTORM localization
     *  process. An Output will receive signals about the
     *  progress of the computation, get an initial notification
     *  about static run properties and receive many EngineResult
     *  structures which contain localizations to process. */
    class Output : public virtual simparm::Node {
      public:
        class Announcement;
        enum AdditionalData { 
            NoData = 0x0, SourceImage = 0x1, SmoothedImage = 0x2, 
            CandidateTree = 0x4, InputBuffer = 0x8,
            LocalizationSources = 0x10, 
            HighestBitInAdditionalData = LocalizationSources };

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
            operator<<(std::ostream &o, AdditionalData data);
        friend std::ostream &
            operator<<(std::ostream &o, Result r);
        friend std::ostream &
            operator<<(std::ostream &o, ProgressSignal r);

      protected:
        /** Method throws an exception when \c can_provide does not
         *  cover \c are_desired, and does nothing otherwise. */
        static void check_additional_data_with_provided
            (std::string name, AdditionalData can_provide, 
                               AdditionalData are_desired)
;
      public:
        virtual ~Output() {}
        Output* clone() const = 0;

        /** This method is called before the rapidSTORM engine is run. It's
         *  parameters are the width and the height of a source image and the
         *  number of source images.
         *  @return A bitfield indicating which additional data (besides the
         *          source image number and the found localizations) should
         *          be transmitted with receiveLocalizations(). */
        virtual AdditionalData announceStormSize(const Announcement&) 
 = 0;
        virtual void propagate_signal(ProgressSignal) = 0;
        virtual Result receiveLocalizations(const EngineResult&)
            = 0;
    };

    /** This structure summarizes the static information
     *  sent at run start. */
    struct Output::Announcement {
        /** X-Width of the input movie */
        int width;     
        /** Y-Height of the input movie */
        int height;    
        /** Z-Depth of the input movie. For now always 1. */
        int depth;     
        /** V-Color-number of the input movie. For now always 1. */
        int colors;    

        /** Number of images in the input movie. */
        int length;    

        /** If the data source knows which carburettor supplies the
          * images, this pointer is set to it, and NULL otherwise. */
        Input* carburettor;
        /** This pointer is set to the last structure able to repeat
          * the delivered results, if any. NULL otherwise. */
        ResultRepeater *result_repeater;

        Announcement(int width, int height, int length,
                     Input* carburettor = NULL,
                     ResultRepeater *repeater = NULL)
        : width(width), height(height), depth(1), colors(1),
          length(length), carburettor(carburettor),
          result_repeater(repeater) {}
    };

    /** An EngineResult structure is sent when localizations were
     *  found and are published to transmissions. */
    struct Output::EngineResult {
        /** Number of the image the localizations were found in. */
        unsigned long forImage;
        /** Pointer to array of localizations. May be NULL if
         *  number is 0, must be given otherwise.*/
        const Localization *first;
        /** Number of localizations in array. */
        int number;
        /** If the SourceImage AdditionalData field was set,
         *  this pointer points to the image the localizations
         *  were computed in. */
        const cimg_library::CImg<StormPixel> *source;
        /** If the SmoothedImage AdditionalData field was set,
         *  this pointer points to the image where candidates
         *  were found in. */
        const cimg_library::CImg<SmoothedPixel> *smoothed;
        /** If the CandidateTree AdditionalData field was set,
         *  this pointer points to the candidate merging tree. */
        const dStorm::CandidateTree<SmoothedPixel> *candidates;
    };

}

#endif
