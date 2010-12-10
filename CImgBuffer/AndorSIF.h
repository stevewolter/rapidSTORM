#ifndef CImgBuffer_ANDORSIF_H
#define CImgBuffer_ANDORSIF_H

#include "SerializedSource.h"
#include "Config.h"
#include "Image.h"
#include "InputMethod.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>

#ifndef CImgBuffer_SIFLOADER_CPP
typedef void readsif_File;
typedef void readsif_DataSet;
#endif

namespace CImgBuffer {
    /** The AndorSIFSource provides images from an Andor SIF file, which
     *  is the file format used by the Andor programs. */
    template <typename PixelType>
    class AndorSIFSource : public SerializedSource< Image<PixelType> >,
                           public ConfigSet
    {
      public:
         AndorSIFSource(const char *src) throw(std::exception);
         AndorSIFSource(FILE *src, const std::string& ident)
            throw(std::exception);
         virtual ~AndorSIFSource() throw();

         virtual int dimx() throw() { return _width; }
         virtual int dimy() throw() { return _height; }
         virtual int image_quantity() throw(); 

         ConfigObject& getConfig() throw() { return *this; }

      private:
         void init(FILE *src) throw(std::exception);

         FILE *stream;
         readsif_File *file;
         readsif_DataSet *dataSet;

         std::string file_ident;
         int _width, _height;

         std::auto_ptr<ConfigSet> sifInfo;

      protected:
         virtual std::auto_ptr< Image<PixelType> > 
            get_image(int image_index) throw(std::exception);

    };

    /** Config class for AndorSIFSource. */
    template <typename PixelType>
    class AndorSIFConfig : public InputConfig< Image<PixelType> > {
      public:
        typedef Image<PixelType> Yield;
        typedef Config<Yield> MasterConfig;
        typedef Config<Yield> YieldConfig;
        typedef std::auto_ptr< InputConfig<Yield> > Result;

        AndorSIFConfig(YieldConfig& src) throw();
        AndorSIFConfig(const AndorSIFConfig &c, YieldConfig& src) throw();

        YieldConfig &master;
        ConfigEntryFile &inputFile;

        std::auto_ptr< Source<Yield> > makeSource() throw(std::exception);
        Result clone(MasterConfig& newMaster) const throw()
            { return Result(new AndorSIFConfig<PixelType>
                                           (*this, newMaster)); }
    };

    /** Input method class for AndorSIFSource. */
    template <typename PixelType>
    class AndorSIF : public InputMethod< Image<PixelType> > {
      public:
        typedef Image<PixelType> Yield;
        typedef std::auto_ptr< InputConfig<Yield> > Result;

        AndorSIF() throw() {}
        ~AndorSIF() throw() {}
        
        virtual Result makeConfig(Config<Yield>& master) throw()
            { return Result(new AndorSIFConfig<PixelType>(master)); }
    };

}

#endif
