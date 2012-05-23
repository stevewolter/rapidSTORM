#ifndef DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_CONFIG_H
#define DSTORM_OUTPUTS_BINNEDLOCALIZATIONS_STRATEGIES_CONFIG_H

#include <simparm/Object.hh>
#include <simparm/ChoiceEntry.hh>
#include "../output/binning/config.h"
#include "../output/binning/binning_decl.h"
#include <boost/ptr_container/ptr_array.hpp>
#include "BinnedLocalizations_decl.h"
#include "../Localization.h"
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace outputs {

template <int Dim>
class DimensionSelector
{
    simparm::Object name_object;
    simparm::BoolEntry invert_y_axis, use_z_axis;

    dStorm::default_on_copy< boost::signals2::signal<void()> > value_change;
    simparm::BaseAttribute::ConnectionStore listening[2];

    boost::ptr_array< output::binning::FieldChoice, Dim+1 > components;
    void init();

  public:
    DimensionSelector();
    ~DimensionSelector();
    DimensionSelector* clone() const { return new DimensionSelector(); }

    std::auto_ptr< BinningStrategy<Dim> > make() const;
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
