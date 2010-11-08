#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/Image_decl.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/chain/Link.h>
#include <dStorm/SizeTraits.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/FileContext.h>

#ifndef DSTORM_TIFFLOADER_CPP
typedef void TIFF;
#endif

namespace dStorm {
  namespace TIFF {
    using namespace dStorm::input;

    struct Config : public simparm::Object {
        simparm::BoolEntry ignore_warnings, determine_length;

        Config();
        void registerNamedEntries() {
            push_back( ignore_warnings ); 
            push_back( determine_length ); 
        }
    };

    class OpenFile;

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
        Source(boost::shared_ptr<OpenFile> file);
        virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits();

        Object& getConfig() { return *this; }
        void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }

      private:
        boost::shared_ptr<OpenFile> file;

        static void TIFF_error_handler(const char*, 
            const char *fmt, va_list ap);
        
        void throw_error();
    };

    class OpenFile : boost::noncopyable {
        ::TIFF *tiff;
        bool ignore_warnings;
        std::string file_ident;

        int current_directory;

        int _width, _height, _no_images;
        dStorm::SizeTraits<2>::Resolution resolution;

        template <typename PixelType> friend class Source<PixelType>::iterator;


      public:
        OpenFile(const std::string& filename, const Config&, simparm::Node&);
        ~OpenFile();

        const std::string for_file() const { return file_ident; }

        template <typename PixelType> 
            std::auto_ptr< Traits<dStorm::Image<PixelType,2> > > 
            getTraits();

        template <typename PixelType>
            std::auto_ptr< dStorm::Image<PixelType,2> >
            load_image( int index );
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    class ChainLink
    : public input::chain::Terminus, protected simparm::Listener
    {
      public:
        ChainLink();
        ChainLink(const ChainLink& o);

        ChainLink* clone() const { return new ChainLink(*this); }
        BaseSource* makeSource();
        AtEnd context_changed( ContextRef, Link* );
        simparm::Node& getNode() { return config; }

      private:
        simparm::Structure<Config> config;
        boost::shared_ptr<OpenFile> file;
        boost::shared_ptr<const input::chain::FileContext> context;

        void open_file();
      protected:
        void operator()(const simparm::Event&);
    };
}

}

#endif
