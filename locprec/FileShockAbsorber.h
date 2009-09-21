#ifndef DSTORM_FILESHOCKABSORBER_H
#define DSTORM_FILESHOCKABSORBER_H

#include "ShockAbsorberConfig.h"
#include "Transmission.h"
#include "Config.h"

namespace dStorm {
class FileShockAbsorber : public ShockAbsorber {
  private:
    const ShockAbsorberConfig &c;
    long int *x, *y;
    int size;
  public:
    FileShockAbsorber(const ShockAbsorberConfig &c);
    ~FileShockAbsorber();
    void editFits(Localization *first, int number, const Image &image);
};
}

#endif
