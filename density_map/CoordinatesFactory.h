#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_CONFIG_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_CONFIG_H

#include <simparm/Object.h>
#include <simparm/ChoiceEntry.h>
#include <dStorm/output/binning/config.h>
#include <dStorm/output/binning/binning_decl.h>
#include <boost/ptr_container/ptr_array.hpp>
#include <dStorm/Localization.h>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace density_map {

template <int Dim> class Coordinates;

template <int Dim>
class CoordinatesFactory
{
    simparm::Object name_object;
    simparm::BoolEntry invert_y_axis, use_z_axis;

    dStorm::default_on_copy< boost::signals2::signal<void()> > value_change;
    simparm::BaseAttribute::ConnectionStore listening[2];

    boost::ptr_array< output::binning::FieldChoice, Dim+1 > components;
    void init();

  public:
    CoordinatesFactory();
    ~CoordinatesFactory();
    CoordinatesFactory* clone() const { return new CoordinatesFactory(); }

    std::auto_ptr< Coordinates<Dim> > make() const;
    std::auto_ptr< output::binning::Unscaled > make_unscaled(int field) const;
    std::auto_ptr< output::binning::Scaled > make_x() const;
    std::auto_ptr< output::binning::Scaled > make_y() const;
    std::auto_ptr< output::binning::Unscaled > make_i() const;
    void set_visibility(const input::Traits<Localization>&);

    bool is_3d() const { return use_z_axis(); }
    void add_listener( simparm::BaseAttribute::Listener& );

    void attach_ui( simparm::NodeHandle at );
};

}
}

#endif
