#include "config.h"
#ifdef HAVE_LIBREADSIF

#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/Config.h>
#include <dStorm/input/FileBasedMethod.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/helpers/thread.h>
#include <dStorm/Image.h>

#ifndef CImgBuffer_SIFLOADER_CPP
typedef void readsif_File;
typedef void readsif_DataSet;
#endif

namespace dStorm {
namespace input {
    class BasenameWatcher;

namespace AndorSIF {
    /** The Source provides images from an Andor SIF file, which
     *  is the file format used by the Andor programs.
     *
     *  The Andor SIF source is parameterized by the output pixel
     *  type, which can be one of unsigned short, unsigned int and
     *  float. This is not necessarily the internal type used by
     *  the SIF file. */
    template <typename PixelType>
    class Source : public simparm::Set,
                   public input::Source< Image<PixelType,2> >,
                   public simparm::Node::Callback
    {
      public:
         typedef dStorm::Image<PixelType,2> Image;
         typedef input::Source<Image> BaseSource;
         typedef typename BaseSource::iterator base_iterator;
         typedef typename BaseSource::Flags Flags;
         typedef typename BaseSource::TraitsPtr TraitsPtr;

         Source(const char *src);
         Source(FILE *src, const std::string& ident);
         virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits();

         Object& getConfig() { return *this; }
         void push_back_SIF_info() {
            push_back( *sifInfo );
         }

      private:
         void init(FILE *src);
         bool has_been_iterated;

         FILE *stream;
         readsif_File *file;
         readsif_DataSet *dataSet;
         bool had_errors;
         int im_count;

         std::string file_ident;

         std::auto_ptr<Set> sifInfo;
         simparm::TriggerEntry showDetails, hideDetails;

         void operator()(const simparm::Event&);

         std::auto_ptr< Image > load(int i);
         class iterator;
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    template <typename PixelType>
    class Config 
    : public FileBasedMethod< dStorm::Image<PixelType,2> >
    {
      public:
        typedef input::Config MasterConfig;

        Config(MasterConfig& src);
        Config(const Config &c, MasterConfig& src);

        Config* clone(MasterConfig& newMaster) const
            { return new Config<PixelType>(*this, newMaster); }

      protected:
        Source< PixelType >* impl_makeSource();
    };
}

}
}

#endif  /* double inclusion prevention */

#endif  /* HAVE_LIBREADSIF */
