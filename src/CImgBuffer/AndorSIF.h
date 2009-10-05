#ifdef HAVE_LIBREADSIF

#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include <CImgBuffer/SerialImageSource.h>
#include <CImgBuffer/Config.h>
#include <CImgBuffer/InputMethod.h>
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/helpers/thread.h>

#ifndef CImgBuffer_SIFLOADER_CPP
typedef void readsif_File;
typedef void readsif_DataSet;
#endif

namespace CImgBuffer {
    class BasenameWatcher;

namespace AndorSIF {
    using cimg_library::CImg;

    /** The Source provides images from an Andor SIF file, which
     *  is the file format used by the Andor programs.
     *
     *  The Andor SIF source is parameterized by the output pixel
     *  type, which can be one of unsigned short, unsigned int and
     *  float. This is not necessarily the internal type used by
     *  the SIF file. */
    template <typename PixelType>
    class Source : public SerialImageSource<PixelType>,
                   public simparm::Set,
                   public  simparm::Node::Callback
    {
      public:
         Source(const char *src);
         Source(FILE *src, const std::string& ident);
         virtual ~Source();

         virtual int dimx() const { return _width; }
         virtual int dimy() const { return _height; }
         virtual int quantity() const; 

         Object& getConfig() { return *this; }
         void push_back_SIF_info() {
            push_back( *sifInfo );
         }

      private:
         void init(FILE *src);

         FILE *stream;
         readsif_File *file;
         readsif_DataSet *dataSet;

         std::string file_ident;
         int _width, _height;

         std::auto_ptr<Set> sifInfo;
         simparm::TriggerEntry showDetails, hideDetails;

         void operator()( Node&, Cause, Node* );

         CImg<PixelType>* load();
    };

    /** Config class for Source. Simple config that adds
     *  the sif extension to the input file element. */
    template <typename PixelType>
    class Config 
    : public InputConfig< CImg<PixelType> >
    {
      public:
        typedef CImgBuffer::Config MasterConfig;

        Config(MasterConfig& src);
        Config(const Config &c, MasterConfig& src);

        MasterConfig &master;
        simparm::FileEntry &inputFile;

        Config* clone() const
            { throw std::logic_error("AndorSIF::Config unclonable."); }
        Config* clone(MasterConfig& newMaster) const
            { return new Config<PixelType>(*this, newMaster); }

      protected:
        Source< PixelType >* impl_makeSource() 
;

      private:
        simparm::Attribute<std::string> sif_extension;
        std::auto_ptr<BasenameWatcher> watcher;
    };
}

}

#endif  /* double inclusion prevention */

#endif  /* HAVE_LIBREADSIF */
