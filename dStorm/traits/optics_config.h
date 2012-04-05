#ifndef DSTORM_TRAITS_OPTICS_CONFIG_H
#define DSTORM_TRAITS_OPTICS_CONFIG_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <dStorm/engine/InputTraits.h>
#include <simparm/Object.hh>
#include <simparm/Callback.hh>
#include <simparm/BoostOptional.hh>
#include "../UnitEntries/PixelSize.h"
#include "../units/nanolength.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Entry_Impl.hh>
#include <simparm/ChoiceEntry_Impl.hh>

#include "position.h"
#include "ProjectionConfig.h"
#include <dStorm/threed_info/Config.h>
#include <dStorm/units/permicrolength.h>
#include <dStorm/Localization_decl.h>

namespace dStorm {
namespace traits {

class PlaneConfig : public simparm::Set {
    const bool is_first_layer;
    typedef Eigen::Matrix< boost::units::quantity<boost::units::si::nanolength, double>, 2, 1, Eigen::DontAlign > ZPosition;
    simparm::Entry< ZPosition > z_position, z_range;
    simparm::Entry< boost::optional<camera_response> > counts_per_photon;
    simparm::Entry< boost::optional< boost::units::quantity<boost::units::camera::intensity, int > > > dark_current;
    simparm::NodeChoiceEntry< ProjectionConfig > alignment;
    typedef boost::ptr_vector< simparm::Entry<double> > Transmissions;
    Transmissions transmissions;
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;
    typedef Eigen::Matrix< boost::units::quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > PixelSize;
    simparm::Entry<PixelSize> pixel_size;

    typedef simparm::Entry< 
        Eigen::Matrix< quantity<si::permicrolength>, Direction_2D, 
                       polynomial_3d::Order, Eigen::DontAlign > > SlopeEntry;
    SlopeEntry slopes;
    simparm::FileEntry z_calibration_file;

    friend class threed_info::No3DConfig;
    friend class threed_info::Polynomial3DConfig;
    friend class threed_info::Spline3DConfig;

    void set_fluorophore_count( int fluorophore_count, bool multiplane );

  public:
    PlaneConfig(int number);
    PlaneConfig( const PlaneConfig& );
    void registerNamedEntries();

    void set_context( const traits::Optics&, int fluorophore_count, bool multilayer, threed_info::Config& );
    void set_context( const input::Traits<Localization>&, int fluorophore_count, threed_info::Config& );
    void write_traits( traits::Optics&, const threed_info::Config& ) const;
    void write_traits( input::Traits<Localization>&, const threed_info::Config& ) const;
    void read_traits( const traits::Optics&, threed_info::Config& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

class CuboidConfig
: public simparm::Object
{
    typedef boost::ptr_vector< PlaneConfig > Layers;
    Layers layers;

    void set_number_of_planes(int);

  public:
    CuboidConfig();
    ~CuboidConfig();
    void registerNamedEntries();

    void set_context( const input::Traits<engine::ImageStack>&, threed_info::Config& );
    void set_context( const input::Traits<Localization>&, threed_info::Config& );
    void read_traits( const input::Traits<engine::ImageStack>&, threed_info::Config& );
    void write_traits( input::Traits<engine::ImageStack>&, const threed_info::Config&) const;
    void write_traits( input::Traits<Localization>&, const threed_info::Config&) const;
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

}
}

#endif
