#ifndef DSTORM_GAUSSFITTER_WIDTH_INVARIANTS_H
#define DSTORM_GAUSSFITTER_WIDTH_INVARIANTS_H

#include <fit++/Exponential2D.hh>
#include <dStorm/engine/JobInfo_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/engine/Image.h>
#include <dStorm/Localization_decl.h>
#include "GaussFitterConfig_decl.h"

namespace dStorm {
namespace engine {

using namespace fitpp;
using namespace fitpp::Exponential2D;

template <int FitFlags, bool Residue_Analysis>
struct Width_Invariants;

template <int FitFlags>
struct Width_Invariants<FitFlags, true>
: public Width_Invariants<FitFlags, false>
{
;
};

}
}

#endif
