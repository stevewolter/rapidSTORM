#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/Image_decl.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/FileInput.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>

#ifndef DSTORM_TIFFLOADER_CPP
typedef void TIFF;
#endif

struct TestState;

namespace dStorm {
namespace TIFF {
    using namespace dStorm::input;
    extern const std::string test_file_name;

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
    template <typename PixelType, int Dimensions>
    class Source : public simparm::Set,
                   public input::Source< Image<PixelType,Dimensions> >
    {
        typedef dStorm::Image<PixelType,Dimensions> Image;
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
        bool ignore_warnings, determine_length;
        std::string file_ident;

        int current_directory;

        int size[3], _no_images;
        traits::Optics<2>::Resolutions resolution;

        template <typename PixelType, int Dim> 
            friend class Source<PixelType,Dim>::iterator;

      public:
        OpenFile(const std::string& filename, const Config&, simparm::Node&);
        ~OpenFile();

        template <typename PixelType, int Dimensions> 
            std::auto_ptr< Traits<dStorm::Image<PixelType,Dimensions> > > 
            getTraits( bool final, simparm::Entry<long>& );
        std::auto_ptr< BaseTraits > getTraits();

        template <typename PixelType, int Dimensions>
            std::auto_ptr< dStorm::Image<PixelType,Dimensions> >
            load_image( int index );
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    class ChainLink
    : public input::FileInput<ChainLink,OpenFile>, protected simparm::Listener
    {
      public:
        ChainLink();

        ChainLink* clone() const { return new ChainLink(*this); }
        simparm::Node& getNode() { return config; }
        BaseSource* makeSource();

      private:
        simparm::Structure<Config> config;
        friend class input::FileInput<ChainLink,OpenFile>;
        OpenFile* make_file( const std::string& ) const;
        void modify_meta_info( chain::MetaInfo& info );

      protected:
        void operator()(const simparm::Event&);
    };

void unit_test( TestState& );

}

}

#endif
