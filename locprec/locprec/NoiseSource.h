#ifndef LOCPREC_NOISESOURCE_H
#define LOCPREC_NOISESOURCE_H

#include <CImgBuffer/InputMethod.h>
#include <CImgBuffer/Image.h>
#include <gsl/gsl_rng.h>
#include <locprec/Fluorophore.h>
#include <locprec/NoiseGenerator.h>

namespace CImgBuffer {
    template <typename PixelType> class Image;
};

namespace locprec {
    template <typename PixelType> class NoiseConfig;

    template <typename PixelType> class NoiseSource 
    : public CImgBuffer::Source< CImgBuffer::Image<PixelType> >,
      public virtual simparm::ConfigSet
    {
      private:
        simparm::ConfigEntryUnsignedLong &randomSeedEntry;
        std::auto_ptr< NoiseGenerator<PixelType> > noiseGenerator;
        int imW, imH, imN;
        std::list<Fluorophore*> fluorophores;

      protected:
        gsl_rng *rng;

      public:
        NoiseSource(NoiseConfig<PixelType> &config) throw();
        ~NoiseSource() throw();

        std::auto_ptr< CImgBuffer::Image<PixelType> > 
            fetch(int index) throw();
        virtual int dimx() throw() { return imW; }
        int dimy() throw() { return imH; }
        int image_quantity() throw() { return imN; }

        const std::list<Fluorophore*>& getFluorophores() const throw()
            { return fluorophores; }
        simparm::ConfigObject& getConfig() throw() { return *this; }
    };

    template <typename PixelType> class NoiseConfig
    : public CImgBuffer::InputConfig< CImgBuffer::Image<PixelType> >
    {
      public:
        FluorophoreConfig fluorophoreConfig;
        NoiseGeneratorConfig noiseGeneratorConfig;

        simparm::ConfigEntryUnsignedLong imageNumber;
        simparm::ConfigEntryUnsignedLong fluorophoreNumber;
        simparm::ConfigEntryChoice fluorophoreDistribution;

        typedef CImgBuffer::Image<PixelType> Image;
        typedef CImgBuffer::Source<Image> Result;
        typedef CImgBuffer::InputConfig<Image> Base;

        NoiseConfig( CImgBuffer::Config<Image> &master ) throw();
        NoiseConfig( const NoiseConfig<PixelType> &copy, 
                     CImgBuffer::Config<Image> &master ) throw();
        ~NoiseConfig() throw() {}
        void registerNamedEntries() throw();

        virtual std::auto_ptr<Result> makeSource() throw(std::exception) 
            { return std::auto_ptr<Result>(
                    new NoiseSource<PixelType>(*this)); }
        virtual std::auto_ptr<Base> clone
            (CImgBuffer::Config<Image> &newMaster) const throw() 
            { return std::auto_ptr<Base>
                    ( new NoiseConfig<PixelType>(*this, newMaster) ); }

        virtual bool may_be_autoloaded() const throw() { return false; }
    };

    template <typename PixelType> class NoiseMethod
    : public CImgBuffer::InputMethod< CImgBuffer::Image<PixelType> >
    {
      public:
        typedef  CImgBuffer::Image<PixelType> Yield;
        typedef CImgBuffer::InputConfig<Yield> Result;

        static void registerMethod() throw();

        NoiseMethod () throw () {}
        virtual ~NoiseMethod () throw() {}

        virtual std::auto_ptr<Result> makeConfig 
            (CImgBuffer::Config<Yield> &master) throw()
            { return std::auto_ptr<Result>(
                       new NoiseConfig<PixelType>(master)); }
    };
}

#endif
