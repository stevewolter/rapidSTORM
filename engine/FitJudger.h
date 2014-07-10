#ifndef DSTORM_FITJUDGER_H
#define DSTORM_FITJUDGER_H

namespace dStorm {
namespace engine {

class FitJudger {
  public:
    virtual ~FitJudger() {}
    virtual FitJudger* clone() const = 0;
    virtual bool is_above_background( double signal_integral_in_photons, double background_in_photons ) const = 0;
};

}
}

#endif
