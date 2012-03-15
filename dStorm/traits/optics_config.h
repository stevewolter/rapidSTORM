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
#include <simparm/Entry_Impl.hh>
#include <simparm/ChoiceEntry_Impl.hh>

#include "position.h"
#include "ProjectionConfig.h"

namespace dStorm {
namespace traits {

class PlaneConfig : public simparm::Set {
    const bool is_first_layer;
    typedef Eigen::Matrix< boost::units::quantity<boost::units::si::nanolength, double>, 2, 1, Eigen::DontAlign > ZPosition;
    simparm::Entry< ZPosition > z_position;
    simparm::Entry< boost::optional<camera_response> > counts_per_photon;
    simparm::Entry< boost::optional< boost::units::quantity<boost::units::camera::intensity, int > > > dark_current;
    simparm::NodeChoiceEntry< ProjectionConfig > alignment;
    typedef boost::ptr_vector< simparm::Entry<double> > Transmissions;
    Transmissions transmissions;
    typedef  Eigen::Matrix< quantity< si::nanolength, double >, 2, 1, Eigen::DontAlign > PSFSize;
    simparm::Entry<PSFSize> psf_size;

  public:
    PlaneConfig(int number);
    PlaneConfig( const PlaneConfig& );
    void registerNamedEntries();

    void write_traits( traits::Optics& ) const;
    void read_traits( const traits::Optics& );
    void set_number_of_fluorophores(int number, bool have_multiple_layers);
    void set_3d_availability(bool);
};

class CuboidConfig
: public simparm::Object
{
    typedef boost::ptr_vector< PlaneConfig > Layers;
    Layers layers;
    typedef Eigen::Matrix< boost::units::quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > PixelSize;
    simparm::Entry<PixelSize> pixel_size;
  
  public:
    CuboidConfig();
    void registerNamedEntries();
    void set_number_of_fluorophores(int number);
    void set_number_of_planes( int );
    void set_3d_availability(bool);
    int number_of_planes() const;

    void write_traits( input::Traits<engine::ImageStack>&) const;
    void read_traits( const input::Traits<engine::ImageStack>& );
    Position::ResolutionType make_localization_traits() const;
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

}
}

#endif
