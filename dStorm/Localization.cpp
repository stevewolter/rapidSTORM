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

Localization::Localization( 
    const Position::Type& position,
    Amplitude::Type strength
)
: position(position), amplitude(strength)
{}

Localization::Localization( const Localization& l )
: position(l.position), frame_number(l.frame_number),
  amplitude(l.amplitude), psf_width(l.psf_width),
  two_kernel_improvement(l.two_kernel_improvement), 
  fit_residues(l.fit_residues), fluorophore(l.fluorophore),
  local_background(l.local_background),
  children(l.children)
{}
              
Localization::~Localization() {}

}
