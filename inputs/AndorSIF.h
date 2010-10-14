#include "config.h"
#ifdef HAVE_LIBREADSIF

#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <dStorm/engine/Input_decl.h>
#include <dStorm/input/ChainLink.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/helpers/thread.h>
#include <dStorm/Image.h>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <simparm/Set.hh>

#include "AndorSIF_OpenFile.h"

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
    class Source : public simparm::Object,
                   public input::Source< Image<PixelType,2> >
    {
      public:
        typedef dStorm::Image<PixelType,2> Image;
        typedef input::Source<Image> BaseSource;
        typedef typename BaseSource::iterator base_iterator;
        typedef typename BaseSource::Flags Flags;
        typedef typename BaseSource::TraitsPtr TraitsPtr;

        Source(boost::shared_ptr<OpenFile> file);
        virtual ~Source();

        base_iterator begin();
        base_iterator end();
        TraitsPtr get_traits();

        Object& getConfig() { return *this; }

      private:
         boost::shared_ptr<OpenFile> file;
         bool has_been_iterated;

         class iterator;
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    template <typename PixelType>
    class Config 
    : public ChainTerminus,
      public simparm::Object
    {
        boost::shared_ptr<OpenFile> file;

      public:
        Config();

        virtual void context_changed( ContextRef );
        virtual Source<PixelType>* makeSource();
        virtual simparm::Node& getNode() { return *this; }

        Config* clone() const { return new Config(*this); }
    };
}

}
}

#endif  /* double inclusion prevention */

#endif  /* HAVE_LIBREADSIF */
