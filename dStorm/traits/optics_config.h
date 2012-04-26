#ifndef DSTORM_TRAITS_OPTICS_CONFIG_H
#define DSTORM_TRAITS_OPTICS_CONFIG_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <dStorm/engine/InputTraits.h>
#include <simparm/Object.hh>
#include <simparm/Set.hh>
#include <simparm/Callback.hh>
#include <simparm/BoostOptional.hh>
#include "../UnitEntries/PixelSize.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>
#include <simparm/Entry_Impl.hh>
#include <simparm/ChoiceEntry_Impl.hh>

#include "position.h"
#include "ProjectionConfig.h"
#include <dStorm/threed_info/Config.h>
#include <dStorm/Localization_decl.h>
#include <dStorm/Direction.h>

namespace dStorm {
namespace traits {

class PlaneConfig : public simparm::Set {
public:
    enum Purpose { InputSimulation, PSFDisplay, FitterConfiguration };
private:
    const Purpose purpose;

    simparm::NodeChoiceEntry< threed_info::Config > three_d;
    simparm::Entry< boost::optional<camera_response> > counts_per_photon;
    simparm::Entry< boost::optional< boost::units::quantity<boost::units::camera::intensity, int > > > dark_current;
    simparm::NodeChoiceEntry< ProjectionConfig > alignment;
    typedef boost::ptr_vector< simparm::Entry<double> > Transmissions;
    Transmissions transmissions;
    typedef Eigen::Matrix< boost::units::quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > PixelSize;
    simparm::Entry<PixelSize> pixel_size;

    void set_fluorophore_count( int fluorophore_count, bool multiplane );

public:
    PlaneConfig(int number, Purpose );
    PlaneConfig( const PlaneConfig& );
    void registerNamedEntries();

    void set_context( const traits::Optics&, int fluorophore_count, bool multilayer );
    void set_context( const input::Traits<Localization>&, int fluorophore_count );
    void write_traits( traits::Optics& ) const;
    void write_traits( input::Traits<Localization>& ) const;
    void read_traits( const traits::Optics& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

class MultiPlaneConfig
: public simparm::Set
{
    typedef boost::ptr_vector< PlaneConfig > Layers;
    Layers layers;

    void set_number_of_planes(int);

    PlaneConfig::Purpose purpose;

  public:
    MultiPlaneConfig(PlaneConfig::Purpose purpose);
    ~MultiPlaneConfig();
    void registerNamedEntries();

    void set_context( const input::Traits<engine::ImageStack>& );
    void set_context( const input::Traits<Localization>& );
    void read_traits( const input::Traits<engine::ImageStack>& );
    void write_traits( input::Traits<engine::ImageStack>&) const;
    void write_traits( input::Traits<Localization>&) const;
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

}
}

#endif
