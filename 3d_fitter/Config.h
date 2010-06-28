#ifndef DSTORM_ENGINE_GAUSS3DFITTERCONFIG_H
#define DSTORM_ENGINE_GAUSS3DFITTERCONFIG_H

#include <simparm/Set.hh>
#include <fitter/MarquardtConfig.h>
#include <fitter/residue_analysis/Config.h>
#include <dStorm/UnitEntries/Nanometre.h>
#include <cs_units/camera/resolution.hpp>

namespace dStorm {
namespace gauss_3d_fitter {

class Config 
: public fitter::MarquardtConfig,
  public fitter::residue_analysis::Config
{
  public:
    Config();
    ~Config();
    void registerNamedEntries();

    dStorm::FloatNanometreEntry z_distance;
    simparm::UnitEntry<cs_units::camera::resolution, float>
        defocus_constant;
};

}
}

#endif
