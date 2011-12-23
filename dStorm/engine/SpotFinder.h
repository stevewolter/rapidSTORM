#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include <simparm/Node.hh>
#include <memory>

#include "Image.h"
#include "CandidateTree.h"
#include "../ImageTraits.h"
#include "../output/Basename_decl.h"
#include "../output/Traits_decl.h"
#include "JobInfo_decl.h"

#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/ptr_container/clone_allocator.hpp>

namespace dStorm {
namespace engine {
namespace spot_finder {

struct Job {
    const double smoothing_mask;
    const InputTraits::Size& size;
    const InputTraits& traits;
    const traits::Optics<2>& optics;
    const FluorophoreTraits& fluorophore;

    Job( double smoothing_mask, const InputTraits& traits, 
         const traits::Optics<2>& optics, const FluorophoreTraits& fluorophore )
        : smoothing_mask(smoothing_mask), 
          size(traits.size), traits(traits), optics(optics), fluorophore(fluorophore) {}

    boost::units::quantity< boost::units::camera::length > sigma(int dimension) const;
};

   /** The SpotFinder class is the base class for all spot 
    *  finding mechanisms. It provides a smoothing buffer image
    *  for storing the noise-reduced image where candidates are
    *  found in, if any is used.
    *
    *  The dStorm engine will acquire a specific implementation
    *  of the SpotFinder class via a SpotFinderFactory class.
    *  The list of all SpotFinderFactory classes is acquired via
    *  the corresponding config element in the dStorm config. */
   class Base {
      protected:
        Base( const Job& );
        const int msx, /**< Smoothing mask radius in X */
                  msy, /**< Smoothing mask radius in Y */
                  bx,  /**< Border (non-smoothed at image border)
                            radius in X */
                  by,  /**< Border (non-smoothed at image border)
                            radius in Y */
                  imw, /**< Width of the smoothed image */
                  imh; /**< Height of the smoothed image */
        /** Buffer that contains the smoothed image. */
        SmoothedImage smoothed; 

      public:
        virtual ~Base();
        virtual Base* clone() const = 0;

        typedef CandidateTree<SmoothedPixel> Candidates;
        virtual void smooth( const Image2D &image ) = 0;
        virtual void findCandidates( Candidates& into );

        const SmoothedImage& getSmoothedImage() const
            { return smoothed; }
   };

    /** The SpotFinderFactory is a generalization of a simparm 
     *  configuration node which is capable of configuring and producing
     *  a SpotFinder object. */
    class Factory 
    {    
        simparm::Node& node;
      public:
        Factory(simparm::Node& node) : node(node) {}
        simparm::Node& getNode() { return node; }
        operator simparm::Node&() { return node; }
        const simparm::Node& getNode() const { return node; }
        operator const simparm::Node&() const { return node; }

        virtual std::auto_ptr<Base> make
            ( const Job& job ) const = 0;
        virtual Factory* clone() const = 0;
        virtual ~Factory() {}
        virtual void set_requirements( input::Traits<engine::Image>& ) {}
        virtual void set_traits( output::Traits&, const JobInfo& ) {}
        virtual void set_variables( output::Basename& ) const {}
    };

    template <typename BaseClass>
    class Builder
        : public BaseClass::Config, public Factory
    {
      public:
        Builder() 
            : Factory(
                static_cast<typename BaseClass::Config&>
                    (*this)) {}
        Builder(const Builder<BaseClass>& o)
            : BaseClass::Config(o), 
              Factory(
                static_cast<typename BaseClass::Config&>
                    (*this)) {}

        virtual Builder<BaseClass>* clone() const 
            { return new Builder<BaseClass>(*this); }
        virtual std::auto_ptr<Base> make(const Job& job) const
            { return std::auto_ptr<Base>(
                new BaseClass( *this, job ) ); }
    };
}
}
}

namespace boost {
template <>
inline dStorm::engine::spot_finder::Base* 
new_clone<dStorm::engine::spot_finder::Base>
    ( const dStorm::engine::spot_finder::Base& o ) 
    { return o.clone(); }
}


#endif
