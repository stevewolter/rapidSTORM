#include "Localization.h"
#include "localization/Traits.h"
#include <boost/units/io.hpp>
#include <boost/fusion/include/iteration.hpp>
#include <Eigen/Core>

namespace dStorm {

struct spacesep_output_streamer {
    std::ostream& l;
    mutable bool first;
    spacesep_output_streamer(std::ostream& l) : l(l), first(true) {}

    template <typename Type>
        std::ostream& operator()( Type& type ) const
        { if ( first ) first = false; else l << " "; return (l << type()); }
};

std::ostream&
operator<<(std::ostream &o, const Localization& loc)
{
    boost::fusion::for_each( loc, spacesep_output_streamer(o) );
    return o << "\n";
}

Localization::Localization() {}

Localization::Localization(const samplepos& position, Amplitude::Type strength)
: position_x(position.x()), position_y(position.y()), position_z(position.z()), amplitude(strength)
{}

Localization::Localization( const Localization& l )
: position_x(l.position_x),
  position_y(l.position_y),
  position_z(l.position_z),
  position_uncertainty_x(l.position_uncertainty_x),
  position_uncertainty_y(l.position_uncertainty_y),
  position_uncertainty_z(l.position_uncertainty_z),
  frame_number(l.frame_number),
  amplitude(l.amplitude),
  psf_width_x(l.psf_width_x),
  psf_width_y(l.psf_width_y),
  two_kernel_improvement(l.two_kernel_improvement), 
  fit_residues(l.fit_residues), fluorophore(l.fluorophore),
  local_background(l.local_background),
  children(l.children)
{}
              
Localization::~Localization() {}

}
