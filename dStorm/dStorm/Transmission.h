#ifndef DSTORM_TRANSMISSION_H
#define DSTORM_TRANSMISSION_H

#include <dStorm/Localization.h>
#include <dStorm/Image.h>
#include <stdexcept>
#include <cc++/thread.h>
#include <ConfigSet.hh>

namespace CImgBuffer {
    template <typename PixelType> class Image;
}

namespace dStorm {
    template <typename PixelType> class CandidateTree;
    class Carburettor;

    struct EngineResult {
        unsigned long forImage;
        const Localization *first;
        int number;
        const cimg_library::CImg<StormPixel> *source;
        const cimg_library::CImg<SmoothedPixel> *smoothed;
        const CandidateTree<SmoothedPixel> *candidates;
        const Carburettor* carburettor;
    };

    class Transmission : public simparm::ConfigSet {
      public:
        enum AdditionalData { 
            NoData = 0x0, SourceImage = 0x1, SmoothedImage = 0x2, 
            CandidateTree = 0x4, InputBuffer = 0x8,
            LocalizationSources = 0x16 };
        enum Result { KeepRunning, RemoveTransmission, RestartEngine,
                        StopEngine };

        Transmission() 
            : ConfigObject("Transmission", "General transmission object") {}
        virtual ~Transmission() throw() {}

        virtual AdditionalData announceStormSize(int, int, int) 
            throw(std::exception)
            { return NoData; }
        virtual Result receiveLocalizations(const EngineResult&) throw() = 0;
        virtual void deleteAllResults() throw() = 0;
        virtual void finish() throw() = 0;
    };

}

#endif
