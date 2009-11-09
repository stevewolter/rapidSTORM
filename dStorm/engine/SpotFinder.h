#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include <simparm/Node.hh>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/engine/CandidateTree.h>
#include <memory>

namespace dStorm {
namespace engine {

   class Config;

   /** The SpotFinder class is the base class for all spot 
    *  finding mechanisms. It provides a smoothing buffer image
    *  for storing the noise-reduced image where candidates are
    *  found in, if any is used.
    *
    *  The dStorm engine will acquire a specific implementation
    *  of the SpotFinder class via a SpotFinderFactory class.
    *  The list of all SpotFinderFactory classes is acquired via
    *  the corresponding config element in the dStorm config. */
   class SpotFinder {
      protected:
        SpotFinder(const Config &conf, int imw, int imh);
        const int msx, /**< Smoothing mask radius in X */
                  msy, /**< Smoothing mask radius in Y */
                  bx,  /**< Border (non-smoothed at image border)
                            radius in X */
                  by,  /**< Border (non-smoothed at image border)
                            radius in Y */
                  imw, /**< Width of the smoothed image */
                  imh; /**< Height of the smoothed image */
        /** Buffer that contains the smoothed image. */
        std::auto_ptr<SmoothedImage> smoothed; 

      public:
        enum SpotFinders { Average, Median, Erosion, Reconstruction,
                           Gaussian };
        virtual ~SpotFinder();

        typedef CandidateTree<SmoothedPixel> Candidates;
        virtual void smooth( const Image &image ) = 0;
        virtual void findCandidates( Candidates& into );

        const SmoothedImage& getSmoothedImage() const
            { return *smoothed; }
   };

    /** The SpotFinderFactory is a generalization of a simparm 
     *  configuration node which is capable of configuring and producing
     *  a SpotFinder object. */
    class SpotFinderFactory 
            : public virtual simparm::Node
    {    
      public:
        virtual std::auto_ptr<SpotFinder> make_SpotFinder
            (const Config &conf, int imw, int imh) const = 0;
        virtual SpotFinderFactory* clone() const = 0;
    };

    template <typename BaseClass>
    class SpotFinderBuilder
        : public SpotFinderFactory, public BaseClass::Config
    {
      public:
        virtual SpotFinderBuilder<BaseClass>* clone() const 
            { return new SpotFinderBuilder<BaseClass>(*this); }
        virtual std::auto_ptr<SpotFinder> make_SpotFinder
            (const Config &conf, int imw, int imh) const 
            { return std::auto_ptr<SpotFinder>(
                new BaseClass( *this, conf, imw, imh ) ); }
    };
}
}

#endif
