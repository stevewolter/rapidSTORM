#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <simparm/BoostUnits.hh>
#include <simparm/BoostOptional.hh>
#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <dStorm/UnitEntries.h>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <boost/units/cmath.hpp>
#include <dStorm/output/Basename_decl.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/Engine.h>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <boost/ptr_container/ptr_vector.hpp>

namespace dStorm {
namespace engine {
   using namespace simparm;

   /** Config entry collection class for the dSTORM engine. */
   class _Config : public Set
   {
      protected:
        void registerNamedEntries();
      public:
        _Config();
        ~_Config();

        IntPixelEntry nms_x, nms_y;

        /** The proportionality factor for the smoothing & NMS mask size */
        Entry<double> maskSizeFactor;

        /** The method to use for spot detection. */
        simparm::NodeChoiceEntry<spot_finder::Factory> spotFindingMethod;

        simparm::Set weights;
        boost::ptr_vector< simparm::Entry<float> > spot_finder_weights;

        /** The method to use for spot fitting. */
        simparm::NodeChoiceEntry< spot_fitter::Factory > spotFittingMethod;
        
        /** Continue fitting until this number of bad fits occured. */
        Entry<unsigned long> motivation;
        /** Amplitude threshold to judge localizations by. */
        simparm::Entry< boost::optional< boost::units::quantity<
            camera::intensity, float> > > amplitude_threshold;

        void addSpotFinder( std::auto_ptr<spot_finder::Factory> factory );
        void addSpotFinder( spot_finder::Factory* factory ) 
            { addSpotFinder(std::auto_ptr<spot_finder::Factory>(factory)); }
        void addSpotFitter( std::auto_ptr<spot_fitter::Factory> factory );
        void addSpotFitter( spot_fitter::Factory* factory ) 
            { addSpotFitter(std::auto_ptr<spot_fitter::Factory>(factory)); }

        void set_variables( output::Basename& ) const;
   };

   class Config : public _Config {
     public:
        Config();
        Config(const Config& c);
        Config* clone() const { return new Config(*this); }
   };

}
}

#endif
