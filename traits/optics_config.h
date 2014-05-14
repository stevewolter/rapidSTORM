#ifndef DSTORM_TRAITS_OPTICS_CONFIG_H
#define DSTORM_TRAITS_OPTICS_CONFIG_H

#include <simparm/Eigen_decl.h>
#include <simparm/BoostUnits.h>
#include <simparm/Eigen.h>
#include "engine/InputTraits.h"
#include <simparm/Object.h>
#include <simparm/TabGroup.h>
#include <simparm/Group.h>
#include <simparm/BoostOptional.h>
#include "UnitEntries/PixelSize.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/ManagedChoiceEntry.h>

#include "localization/Fields.h"
#include "traits/ProjectionConfig.h"
#include "threed_info/Config.h"
#include "Localization_decl.h"
#include "Direction.h"

#include <boost/signals2/signal.hpp>
#include "helpers/default_on_copy.h"

namespace dStorm {
namespace traits {

class PlaneConfig {
public:
    enum Purpose { InputSimulation, PSFDisplay, FitterConfiguration };
private:
    simparm::Object name_object;
    simparm::NodeHandle current_ui;
    const Purpose purpose;

    simparm::ManagedChoiceEntry< threed_info::Config > three_d;
    simparm::Entry< boost::optional<camera_response> > counts_per_photon;
    simparm::Entry< boost::optional< boost::units::quantity<boost::units::camera::intensity, int > > > dark_current;
    simparm::ManagedChoiceEntry< ProjectionConfig > alignment;

    struct TransmissionEntry;
    typedef boost::ptr_vector< TransmissionEntry > Transmissions;
    Transmissions transmissions;
    typedef Eigen::Matrix< boost::units::quantity< nanometer_pixel_size, float >, 2, 1, Eigen::DontAlign > PixelSize;
    simparm::Entry<PixelSize> pixel_size;

    void set_fluorophore_count( int fluorophore_count, bool multiplane );

    dStorm::default_on_copy< boost::signals2::signal<void()> > ui_element_changed;
    simparm::BaseAttribute::ConnectionStore listening[5];

public:
    PlaneConfig(int number, Purpose );
    PlaneConfig( const PlaneConfig& );
    ~PlaneConfig();
    void attach_ui( simparm::NodeHandle at );

    void notify_on_any_change( boost::signals2::slot<void()> listener ) 
        { ui_element_changed.connect(listener); }

    void set_context( const traits::Optics&, int fluorophore_count, bool multilayer );
    void write_traits( traits::Optics& ) const;
    void read_traits( const traits::Optics& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

class MultiPlaneConfig
{
    simparm::TabGroup name_object;
    simparm::NodeHandle current_ui;
    typedef boost::ptr_vector< PlaneConfig > Layers;
    Layers layers;

    dStorm::default_on_copy< boost::signals2::signal<void()> > ui_element_listener;

    void set_number_of_planes(int);

    PlaneConfig::Purpose purpose;

  public:
    MultiPlaneConfig(PlaneConfig::Purpose purpose);
    MultiPlaneConfig(const MultiPlaneConfig&);
    ~MultiPlaneConfig();
    void attach_ui( simparm::NodeHandle at );

    void notify_on_any_change( boost::signals2::slot<void()> listener )
        { ui_element_listener.connect(listener); }

    void set_context( const input::Traits<engine::ImageStack>& );
    void read_traits( const input::Traits<engine::ImageStack>& );
    void write_traits( input::Traits<engine::ImageStack>&) const;
    image::MetaInfo<2>::Resolutions get_resolution() const;

    void show() { name_object.show(); }
    void hide() { name_object.hide(); }
    bool ui_is_attached();
};

}
}

#endif
