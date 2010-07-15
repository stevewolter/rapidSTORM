#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/Image_decl.h>
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
                   public input::Source< Image<PixelType,2> >
    {
        typedef dStorm::Image<PixelType,2> Image;
        typedef input::Source<Image> BaseSource;
        typedef typename BaseSource::iterator base_iterator;
        typedef typename BaseSource::Flags Flags;
        typedef typename BaseSource::TraitsPtr TraitsPtr;

        class iterator;

      public:
        Source(const char *src, bool ignore_warnings);
        virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits();

        Object& getConfig() { return *this; }

      private:
        ::TIFF *tiff;
        int current_directory;
        std::string filename;
        bool ignore_warnings;

        int _width, _height, _no_images;
        dStorm::SizeTraits<2>::Resolution resolution;

        static void TIFF_error_handler(const char*, 
            const char *fmt, va_list ap);
        
        void throw_error();
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    template <typename PixelType>
    class Config 
    : public FileBasedMethod< Image<PixelType,2> >
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
