#ifndef CImgBuffer_CIMGLIST_H
#define CImgBuffer_CIMGLIST_H

#include "ImageTraits.h"
#include "Source.h"
#include "Config.h"
#include "InputMethod.h"
#include <memory>
#include <string>
#include <stdexcept>
#include <stdio.h>
#include <simparm/FileEntry.hh>
#include <CImg.h>
#include <stdint.h>

namespace CImgBuffer {
  /** The CImgList namespace provides sources that operate on
   *  a standard CImgList structure. The provided filename is
   *  loaded by CImgList::load() and images in the resulting
   *  sequence are returned one by one. */
  namespace CImgList {
    using cimg_library::CImg;
    using cimg_library::CImgList;

    template <typename PixelType> std::string ident();

    template <typename PixelType>
    class Source : public CImgBuffer::Source< CImg<PixelType> >,
                   public simparm::Object
    {
      public:
         Source(const char *src );
         virtual ~Source() {}

         virtual int dimx() const 
            { return sourceImages.front().width; }
         virtual int dimy() const 
            { return sourceImages.front().height; }
         virtual int quantity() const 
            { return sourceImages.size; }

         Object& getConfig() { return *this; }

      private:
         CImgList<PixelType> sourceImages;

      protected:
        virtual CImg<PixelType>*
            fetch(int image_index)
        {
            return new CImg<PixelType>(sourceImages[image_index]);
        }

    };

    template <typename PixelType>
    class Config : public InputConfig< CImg<PixelType> > {
      public:
        typedef CImg<PixelType> Yield;
        typedef CImgBuffer::Config MasterConfig;

        Config(MasterConfig& src)
            : simparm::Object("CImgList" + ident<PixelType>(),
                              "List of CImg objects"),
              inputFile(src.inputFile) { this->push_back(src.inputFile); }
        Config(const Config &c, MasterConfig& src) 
            : simparm::Object(c), InputConfig< CImg<PixelType> >(c),
              inputFile(src.inputFile) { this->push_back(src.inputFile); }

        simparm::FileEntry &inputFile;

        Config* clone(MasterConfig& newMaster) const
            { return new Config<PixelType>(*this, newMaster); }

      protected:
        CImgBuffer::Source<Yield>* impl_makeSource()
            { return new Source<PixelType>( inputFile().c_str() ); }
    };

  }
}

#endif
