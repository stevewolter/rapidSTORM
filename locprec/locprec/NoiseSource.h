#ifndef LOCPREC_NOISESOURCE_H
#define LOCPREC_NOISESOURCE_H

#include <CImgBuffer/InputMethod.h>
#include <CImgBuffer/ImageTraits.h>
#include <gsl/gsl_rng.h>
#include <locprec/Fluorophore.h>
#include <locprec/FluorophoreDistribution.h>
#include <locprec/NoiseGenerator.h>
#include <cc++/thread.h>

#include <simparm/ChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>

namespace CImgBuffer {
    template <typename PixelType> class Image;
};

namespace locprec {
    template <typename PixelType> class NoiseConfig;

    template <typename PixelType> class NoiseSource 
    : public CImgBuffer::Source< dStorm::Image >,
      public simparm::Set
    {
      private:
        simparm::UnsignedLongEntry &randomSeedEntry;
        std::auto_ptr< NoiseGenerator<PixelType> > noiseGenerator;
        int imW, imH, imN;
        std::list<Fluorophore*> fluorophores;
        ost::Mutex mutex;

        std::auto_ptr<std::ostream> output;

      protected:
        gsl_rng *rng;

      public:
        NoiseSource(NoiseConfig<PixelType> &config);
        ~NoiseSource();

        dStorm::Image* fetch(int index);
        int dimx() const { return imW; }
        int dimy() const { return imH; }
        int quantity() const { return imN; }

        const std::list<Fluorophore*>& getFluorophores() const
            { return fluorophores; }
        simparm::Object& getConfig() { return *this; }
    };

    template <typename PixelType> class NoiseConfig
    : public CImgBuffer::InputConfig< CImgBuffer::Image<PixelType> >
    {
      public:
        FluorophoreConfig fluorophoreConfig;
        NoiseGeneratorConfig noiseGeneratorConfig;

        simparm::UnsignedLongEntry imageNumber;
        simparm::NodeChoiceEntry<FluorophoreDistribution>
            distribution;
        simparm::FileEntry saveActivity;
        simparm::FileEntry store, recall;

        typedef CImgBuffer::Image<PixelType> Image;
        typedef CImgBuffer::Source<Image> Result;

        NoiseConfig( CImgBuffer::Config &master );
        NoiseConfig( const NoiseConfig<PixelType> &copy, 
                     CImgBuffer::Config &master );
        ~NoiseConfig() {}
        void registerNamedEntries();

        virtual NoiseSource<PixelType>* impl_makeSource()
 
            { return new NoiseSource<PixelType>(*this); }
        virtual NoiseConfig<PixelType>* clone
            (CImgBuffer::Config &newMaster) const 
            { return ( new NoiseConfig<PixelType>(*this, newMaster) ); }

        virtual bool may_be_autoloaded() const { return false; }
    };
}

#endif
