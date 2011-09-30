#ifndef LOCPREC_BIPLANE_ALIGNMENT_DECL_H
#define LOCPREC_BIPLANE_ALIGNMENT_DECL_H

namespace locprec {
namespace biplane_alignment {

struct MotionModel;
struct Config;
struct Source;

std::auto_ptr< dStorm::input::chain::Filter > make_filter();

}
}

#endif
