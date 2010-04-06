#ifndef DSTORM_ENGINE_GAUSSFITTERCONFIG_H
#define DSTORM_ENGINE_GAUSSFITTERCONFIG_H

#include <simparm/Set.hh>

namespace dStorm {
namespace engine {

class GaussFitterConfig : public simparm::Set {
  public:
    GaussFitterConfig();
    void registerNamedEntries() {}
};

}
}

#endif
