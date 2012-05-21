#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include <simparm/Node.hh>
#include <memory>

#include "Image.h"
#include "CandidateTree.h"
#include "InputTraits.h"
#include "../output/Basename_decl.h"
#include "../output/Traits_decl.h"
#include "JobInfo_decl.h"

#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <dStorm/make_clone_allocator.hpp>

namespace dStorm {
namespace engine {
namespace spot_finder {

class Job {
    const InputPlane& traits;
    const FluorophoreTraits& fluorophore;

  public:
    Job( const InputPlane& traits, const FluorophoreTraits& fluorophore )
        : traits(traits), fluorophore(fluorophore) {}

    ImageTypes<2>::Size size() const;
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
      public:
        virtual std::auto_ptr<Base> make
            ( const Job& job ) const = 0;
        virtual Factory* clone() const = 0;
        virtual ~Factory() {}
        virtual void set_requirements( InputTraits& ) {}
        virtual void set_traits( output::Traits&, const JobInfo& ) {}
        virtual void set_variables( output::Basename& ) const {}
        virtual void attach_ui( simparm::Node& to ) = 0;
        virtual void detach_ui( simparm::Node& to ) = 0;
        virtual std::string getName() const = 0;
    };

}
}
}

DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::engine::spot_finder::Base)
DSTORM_MAKE_BOOST_CLONE_ALLOCATOR(dStorm::engine::spot_finder::Factory)


#endif
