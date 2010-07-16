#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/input/SerialSource.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/Method.h>
#include <dStorm/input/FileBasedMethod.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>

#ifndef DSTORM_TIFFLOADER_CPP
typedef void TIFF;
#endif

namespace dStorm {
  namespace TIFF {
    using cimg_library::CImg;
    using namespace dStorm::input;

    /** The Source provides images from a TIFF file.
     *
     *  The TIFF source is parameterized by the output pixel
     *  type, which can be one of unsigned char, unsigned short,
     *  unsigned int and float. The loaded TIFF file must match
     *  this data type exactly; no up- or downsampling is per-
     *  formed. */
    template <typename PixelType>
    class Source : public simparm::Set,
                   public SerialSource< CImg<PixelType> >
    {
      public:
        Source(const char *src, bool ignore_warnings);
        virtual ~Source();

         virtual int quantity() const { return _no_images; }

         Object& getConfig() { return *this; }

         void throw_error();

      private:
        ::TIFF *tiff;
        bool ignore_warnings;

        int _width, _height, _no_images;

        static void TIFF_error_handler(const char*, 
            const char *fmt, va_list ap);
        
        cimg_library::CImg<PixelType>* load();
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    template <typename PixelType>
    class Config 
    : public FileBasedMethod< CImg<PixelType> >
    {
      public:
        typedef input::Config MasterConfig;

        Config(MasterConfig& src);
        Config(const Config &c, MasterConfig& src);

        Config* clone(MasterConfig& newMaster) const
            { return new Config<PixelType>(*this, newMaster); }

      protected:
        Source< PixelType >* impl_makeSource();

      private:
        simparm::Attribute<std::string> tiff_extension;
        simparm::BoolEntry ignore_warnings;
    };
}

}

#endif
