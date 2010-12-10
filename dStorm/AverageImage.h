#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/Transmission.h>
#include <dStorm/Image.h>
#include <memory>
#include <CImg.h>

namespace dStorm {
    /** The AverageImage class averages all incoming images into a
     *  single image to show the usefulness of dSTORM. */
    class AverageImage : public Transmission {
      private:
        std::string filename;
        cimg_library::CImg<unsigned long> image;

      public:
        AverageImage(const std::string &filename) throw() 
            : ConfigObject("AverageImage", "Image averaging status"),
              filename(filename) {}
        ~AverageImage() throw() {}

        AdditionalData announceStormSize(int w, int h, int) throw() {
            image.resize(w,h,1,1);
            image.fill(0);
            return SourceImage; 
        }
        Result receiveLocalizations(const EngineResult&) throw();
        void deleteAllResults() throw() { image.fill(0); }
        void finish() throw();

        const char *getName() throw() { return "AverageImage"; }
    };
}
#endif
