#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <simparm/Set.hh>
#include <simparm/Entry.hh>
#include <simparm/OptionalEntry.hh>
#include <dStorm/UnitEntries.h>
#include <simparm/NumericEntry.hh>
#include <simparm/ChoiceEntry.hh>
#include <boost/units/cmath.hpp>
#include <dStorm/output/Basename_decl.h>
#include <dStorm/input/chain/Context.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <dStorm/Engine.h>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/output/LocalizedImage_decl.h>
#include <dStorm/input/Source.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>

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
        DoubleEntry maskSizeFactor;
        /** The proportionality factor for the fitting mask size (the fitting
        *  mask determines which pixel are fitted with the exponential PSF
        *  model). */
        DoubleEntry fitSizeFactor;

        /** The method to use for spot detection. */
        simparm::NodeChoiceEntry<spot_finder::Factory> spotFindingMethod;
        /** The method to use for spot fitting. */
        simparm::NodeChoiceEntry< spot_fitter::Factory > spotFittingMethod;

        /** Continue fitting until this number of bad fits occured. */
        UnsignedLongEntry motivation;
        /** Amplitude threshold to judge localizations by. */
        simparm::OptionalEntry< boost::units::quantity<
            camera::intensity, float> > amplitude_threshold;

        void addSpotFinder( std::auto_ptr<spot_finder::Factory> factory );
        void addSpotFinder( spot_finder::Factory* factory ) 
            { addSpotFinder(std::auto_ptr<spot_finder::Factory>(factory)); }
        void addSpotFitter( std::auto_ptr<spot_fitter::Factory> factory );
        void addSpotFitter( spot_fitter::Factory* factory ) 
            { addSpotFitter(std::auto_ptr<spot_fitter::Factory>(factory)); }

        void set_variables( output::Basename& ) const;

        boost::shared_ptr<input::chain::Context> makeContext() const;
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
