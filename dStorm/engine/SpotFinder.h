#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include <simparm/Node.hh>
#include <memory>

#include "Image_decl.h"
#include "CandidateTree.h"
#include "Config_decl.h"

namespace dStorm {
namespace engine {

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
    {    
        simparm::Node& node;
      public:
        SpotFinderFactory(simparm::Node& node) : node(node) {}
        simparm::Node& getNode() { return node; }
        operator simparm::Node&() { return node; }
        const simparm::Node& getNode() const { return node; }
        operator const simparm::Node&() const { return node; }

        virtual std::auto_ptr<SpotFinder> make_SpotFinder
            (const Config &conf, int imw, int imh) const = 0;
        virtual SpotFinderFactory* clone() const = 0;
        virtual ~SpotFinderFactory() {}
    };

    template <typename BaseClass>
    class SpotFinderBuilder
        : public BaseClass::Config, public SpotFinderFactory
    {
      public:
        SpotFinderBuilder() 
            : SpotFinderFactory(
                static_cast<typename BaseClass::Config&>
                    (*this)) {}
        SpotFinderBuilder(const SpotFinderBuilder<BaseClass>& o)
            : BaseClass::Config(o), 
              SpotFinderFactory(
                static_cast<typename BaseClass::Config&>
                    (*this)) {}

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
