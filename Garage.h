#ifndef DSTORM_GARAGE_H
#define DSTORM_GARAGE_H

#include <simparm/Node.hh>
#include <dStorm/output/Localizations.h>
#include <stdexcept>
#include "MasterConfig.h"

namespace dStorm {
class Carburettor;
class GarageConfig;

class Garage  {
  private:
    std::auto_ptr<GarageConfig> autoConfig;
    GarageConfig& config;

    /** Initialization code common to all constructors. */
    void init() throw();

    void _drive();

  public:
    /** Automatic constructor. */
    Garage( int argc, char *argv[] );
    Garage(GarageConfig& config);
    ~Garage();
};

}

#endif
