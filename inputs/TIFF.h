#ifndef CImgBuffer_TIFF_H
#define CImgBuffer_TIFF_H

#include <dStorm/image/fwd.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/input/FileInput.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/input/Source.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/Entry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Structure.hh>
#include <simparm/Set.hh>

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
    class Source : public simparm::Set,
                   public input::Source< engine::ImageStack >
    {
        typedef engine::StormPixel Pixel;
        typedef engine::ImageStack Image;
        typedef engine::Image2D Plane;
        typedef input::Source<engine::ImageStack> BaseSource;
        typedef typename BaseSource::iterator base_iterator;
        typedef typename BaseSource::TraitsPtr TraitsPtr;

        simparm::Node& node() { return *this; }

      public:
        class iterator;
        Source(boost::shared_ptr<OpenFile> file);
        virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits(typename BaseSource::Wishes);

        Object& getConfig() { return *this; }
        void dispatch(typename BaseSource::Messages m) { assert( ! m.any() ); }
        typename BaseSource::Capabilities capabilities() const 
            { return typename BaseSource::Capabilities(); }

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
        image::MetaInfo<2>::Resolutions resolution;

        friend class Source::iterator;

      public:
        OpenFile(const std::string& filename, const Config&, simparm::Node&);
        ~OpenFile();

        std::auto_ptr< Traits<engine::ImageStack> > 
            getTraits( bool final, simparm::Entry<long>& );
        std::auto_ptr< BaseTraits > getTraits();

        std::auto_ptr< engine::ImageStack >
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
        BaseSource* makeSource();

      private:
        simparm::Structure<Config> config;
        friend class input::FileInput<ChainLink,OpenFile>;
        OpenFile* make_file( const std::string& ) const;
        void modify_meta_info( MetaInfo& info );
        simparm::Object& getNode() { return config; }

      protected:
        void operator()(const simparm::Event&);
    };

void unit_test( TestState& );

}

}

#endif
