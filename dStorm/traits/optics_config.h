#ifndef DSTORM_TRAITS_OPTICS_CONFIG_H
#define DSTORM_TRAITS_OPTICS_CONFIG_H

#include <simparm/Eigen_decl.hh>
#include <simparm/BoostUnits.hh>
#include <simparm/Eigen.hh>
#include <dStorm/engine/InputTraits.h>
#include <simparm/Object.hh>
#include <simparm/Set.hh>
#include <simparm/BoostOptional.hh>
#include "../UnitEntries/PixelSize.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include <simparm/Set.hh>
#include <simparm/Entry_Impl.hh>
#include <simparm/ManagedChoiceEntry.hh>
#include <simparm/ChoiceEntry_Impl.hh>

#include "position.h"
#include "ProjectionConfig.h"
#include <dStorm/threed_info/Config.h>
#include <dStorm/Localization_decl.h>
#include <dStorm/Direction.h>

#include <boost/signals2/signal.hpp>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace traits {

class PlaneConfig {
public:
    enum Purpose { InputSimulation, PSFDisplay, FitterConfiguration };
private:
    simparm::Set name_object;
    boost::optional< simparm::NodeRef > current_ui;
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
    void attach_ui( simparm::Node& at );

    void notify_on_any_change( boost::signals2::slot<void()> listener ) 
        { ui_element_changed.connect(listener); }

    void set_context( const traits::Optics&, int fluorophore_count, bool multilayer );
    void set_context( const input::Traits<Localization>&, int fluorophore_count );
    void write_traits( traits::Optics& ) const;
    void write_traits( input::Traits<Localization>& ) const;
    void read_traits( const traits::Optics& );
    image::MetaInfo<2>::Resolutions get_resolution() const;
};

class MultiPlaneConfig
{
    simparm::Set name_object;
    boost::optional< simparm::NodeRef > current_ui;
    typedef boost::ptr_vector< PlaneConfig > Layers;
    Layers layers;

    dStorm::default_on_copy< boost::signals2::signal<void()> > ui_element_listener;

    void set_number_of_planes(int);

    PlaneConfig::Purpose purpose;

  public:
    MultiPlaneConfig(PlaneConfig::Purpose purpose);
    ~MultiPlaneConfig();
    void attach_ui( simparm::Node& at );

    void notify_on_any_change( boost::signals2::slot<void()> listener )
        { ui_element_listener.connect(listener); }

    void set_context( const input::Traits<engine::ImageStack>& );
    void set_context( const input::Traits<Localization>& );
    void read_traits( const input::Traits<engine::ImageStack>& );
    void write_traits( input::Traits<engine::ImageStack>&) const;
    void write_traits( input::Traits<Localization>&) const;
    image::MetaInfo<2>::Resolutions get_resolution() const;

    void show() { name_object.viewable = true; }
    void hide() { name_object.viewable = false; }
    bool ui_is_attached() { return current_ui && current_ui->isActive(); }
};

}
}

#endif
